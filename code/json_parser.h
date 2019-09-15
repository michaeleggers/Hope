#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdlib.h>

typedef struct JsonToken JsonToken;
typedef struct JsonObject JsonObject;

enum JsonType
{
    JSON_NONE,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_NUMBER,
    JSON_STRING,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL,
    
    JSON_OBJECT_CLOSE,
    JSON_ARRAY_CLOSE,
    JSON_COMMA
};

struct JsonToken
{
    JsonType type;
    
    union
    {
        char name[256];
        float f_num;
    };
    
    int size;
};

struct JsonDocument
{
    JsonToken * values;
    int capacity;
    int size;
};

struct JsonNode
{
    JsonToken * val;
    JsonNode * child;
    JsonNode * parent;
    int count;
    int capacity;
};

void print_tokens(JsonDocument * doc);

#ifdef JSON_PARSER_IMPLEMENTATION

static JsonDocument document;

void json_add(JsonToken val)
{
    if (document.size >= document.capacity) {
        JsonToken * newValues = (JsonToken *)realloc(document.values, 2*document.capacity*sizeof(JsonToken));
        if (!newValues) {
            fprintf(stderr, "failed to realloc in %s at line %d\n\n", __FILE__, __LINE__);
        }
        else {
            document.values = newValues;
            document.capacity *= 2;
        }
    }
    document.values[document.size++] = val;
}

int json_skipWhitespaces(char * buffer)
{
    int skipped = 0;
    while (*buffer == ' ' || *buffer == '\t') { 
        buffer++;
        skipped++; 
    }
    return skipped;
}

void print_indent(int indentationCount)
{
    for (int i=0; i<indentationCount; ++i)
        printf(" ");
}

int skipLine(char * buffer)
{
    int skipped = 0;
    while (*buffer == '\n' || *buffer == '\r') { buffer++; skipped++; }
    return skipped;
}

int json_name(char * out_name, char * buffer)
{
    int length = 0;
    while (*buffer != '"') {
        *out_name++ = *buffer++;
        length++;
    }
    *out_name = '\0';
    while (*buffer == '"') {buffer++; length++;}
    return length;
}

int json_number(char * out_number, char * buffer)
{
    int length = 0;
    while (*buffer >= '0' && *buffer <= '9') {
        *out_number++ = *buffer++;
        length++;
    }
    *out_number = '\0';
    return length;
}

int skip_whitespaces_and_linebreaks(char ** buffer)
{
    int skipped = 0;
    while (**buffer == '\n' || 
           **buffer == '\r' ||
           **buffer == ' ' ||
           **buffer == '\t') { 
        (*buffer)++;
        skipped++;
    }
    return skipped;
}

int advance_to_next_whitespace(char ** buffer)
{
    int skipped = 0;
    while (**buffer != ' ') {
        (*buffer)++;
        skipped++;
    }
    return skipped;
}

static char * buf;
static int indent;

JsonToken json_get_token()
{
    skip_whitespaces_and_linebreaks(&buf);
    JsonToken token;
    switch (*buf)
    {
        case '{':
        {
            token.type = JSON_OBJECT;
            token.size = 0;
        }
        break;
        
        case '}':
        {
            token.type = JSON_OBJECT_CLOSE;
            token.size = 0;
        }
        break;
        
        case '[':
        {
            token.type = JSON_ARRAY;
            token.size = 0;
        }
        break;
        
        case ']':
        {
            token.type = JSON_ARRAY_CLOSE;
            token.size = 0;
        }
        break;
        
        case '"':
        {
            buf++;
            token.type = JSON_STRING;
            token.size = 0;
            buf += json_name(token.name, buf);
        }
        break;
        
        case 'f':
        {
            token.type = JSON_FALSE;
            token.size = 0;
            advance_to_next_whitespace(&buf);
        }
        break;
        
        case 't':
        {
            token.type = JSON_TRUE;
            token.size = 0;
            advance_to_next_whitespace(&buf);
        }
        break;
        
        case ',':
        {
            token.type = JSON_COMMA;
            token.size = 0;
            advance_to_next_whitespace(&buf);
        }
        break;
        
        default:
        {
            if (*buf >= '0' && *buf <= '9') {
                char asciiNumber[32];
                buf += json_number(asciiNumber, buf);
                token.type = JSON_NUMBER;
                token.f_num = atof(asciiNumber);
                advance_to_next_whitespace(&buf);
            }
        }
    }
    buf++;
    return token;
}

void print_token(JsonToken * token)
{
    switch (token->type)
    {
        case JSON_OBJECT:
        {
            print_indent(indent);
            printf("OBJECT\n");
            indent += 2;
        }
        break;
        
        case JSON_OBJECT_CLOSE:
        {
            indent -= 2;
            print_indent(indent);
            printf("OBJECT-CLOSE\n");
        }
        break;
        
        case JSON_ARRAY:
        {
            print_indent(indent);
            printf("ARRAY\n");
            indent += 2;
        }
        break;
        
        case JSON_ARRAY_CLOSE:
        {
            indent -= 2;
            print_indent(indent);
            printf("ARRAY-CLOSE\n");
        }
        break;
        
        case JSON_STRING:
        {
            print_indent(indent);
            printf("%s\n", token->name);
        }
        break;
        
        case JSON_NUMBER:
        {
            print_indent(indent);
            printf("%f\n", token->f_num);
        }
        break;
        
        case JSON_TRUE:
        {
            print_indent(indent);
            printf("TRUE\n");
        }
        break;
        
        case JSON_FALSE:
        {
            print_indent(indent);
            printf("FALSE\n");
        }
        break;
        
        case JSON_COMMA:
        {
            print_indent(indent);
            printf(",\n");
        }
        break;
    }
}

void json_parse(char * buffer)
{
    // TODO(Michael): assert buffer
    
    // print buffer to console for debugging
    char * bufferPos = buffer;
    while (*bufferPos != '\0')
    {
        int skipped = json_skipWhitespaces(bufferPos);
        bufferPos += skipped;
        printf("%c", *bufferPos);
        bufferPos++;
    }
    
    buf = buffer;
    while (*buf != '\0') {
        JsonToken token = json_get_token();
        print_token(&token);
    }
    
}

JsonNode * new_json_node(JsonNode * node, JsonToken * val)
{
    JsonNode newNode;
    newNode.count = 0;
    newNode.capacity = 10;
    newNode.child = (JsonNode *)malloc(newNode.capacity * sizeof(JsonNode));
    newNode.parent = node;
    newNode.val = val;
    if (node->count < node->capacity) {
        node->child[node->count++] = newNode;
    }
    else {
        JsonNode * newChild = (JsonNode *)realloc(node->child, 2*node->capacity*sizeof(JsonNode));
        if (!newChild) {
            fprintf(stderr, "failed to realloc in %s at line %d\n\n", __FILE__, __LINE__);
        }
        else {
            node->child = newChild;
            node->capacity *= 2;
            node->child[node->count++] = newNode;
        }
    }
    
    if (node->count > 0) {
        return &node->child[node->count - 1];
    }
    else {
        return &node->child[0];
    }
}

void print_tokens(JsonDocument * doc)
{
#if 0
    FILE * graphvizfile = fopen("indy_json_ast.dot", "rw");
    fprintf(graphvizfile, "digraph D {\n");
#endif
    int indent = 0;
    JsonToken * value = doc->values;
    printf("document-size: %d\n\n", doc->size);
    JsonNode rootNode;
    rootNode.count = 0;
    rootNode.capacity = 10;
    rootNode.child = (JsonNode *)malloc(rootNode.capacity * sizeof(JsonNode));
    memset((void *)rootNode.child, 0, rootNode.capacity * sizeof(JsonNode));
    rootNode.parent = 0;
    JsonNode * currentNode = &rootNode;
    for (int i=0; i<doc->size; ++i)
    {
        switch (value->type)
        {
            case JSON_OBJECT:
            {
                print_indent(indent);
                printf("OBJECT\n");
                indent += 2;
                
                currentNode = new_json_node(currentNode, value);
            }
            break;
            
            case JSON_OBJECT_CLOSE:
            {
                indent -= 2;
                print_indent(indent);
                printf("OBJECT-CLOSE\n");
                
                currentNode = currentNode->parent;
            }
            break;
            
            case JSON_ARRAY:
            {
                print_indent(indent);
                printf("ARRAY\n");
                indent += 2;
                
                currentNode = new_json_node(currentNode, value);
            }
            break;
            
            case JSON_ARRAY_CLOSE:
            {
                indent -= 2;
                print_indent(indent);
                printf("ARRAY-CLOSE\n");
                
                currentNode = currentNode->parent;
            }
            break;
            
            case JSON_STRING:
            {
                print_indent(indent);
                printf("%s\n", value->name);
#if 0
                currentNode = new_json_node(currentNode, value);
#endif
            }
            break;
            
            case JSON_NUMBER:
            {
                print_indent(indent);
                printf("%f\n", value->f_num);
            }
            break;
            
            case JSON_TRUE:
            {
                print_indent(indent);
                printf("TRUE\n");
            }
            break;
            
            case JSON_FALSE:
            {
                print_indent(indent);
                printf("FALSE\n");
            }
            break;
            
            case JSON_COMMA:
            {
                print_indent(indent);
                printf(",\n");
            }
            break;
        }
        value++;
    }
    //fclose(graphvizfile);
}

#endif

#endif