#ifndef JSON_PARSER_H
#define JSON_PARSER_H


typedef struct JsonValue JsonValue;
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
    JSON_ARRAY_CLOSE
};

struct JsonValue
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
    JsonValue * values;
    JsonType * stack;
    int stackPos;
    int capacity;
    int size;
};

void print_tokens(JsonDocument * doc);

#ifdef JSON_PARSER_IMPLEMENTATION

static JsonDocument document;

void json_add(JsonValue val)
{
    document.values[document.size++] = val;
}

void json_push(JsonType type)
{
    document.stack[document.stackPos++] = type;
}

void json_pop()
{
    JsonValue * val = &document.values[--document.stackPos];
    val->size = document.size - document.stackPos;
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

JsonDocument parse_json(char * buffer)
{
    document.size = 0;
    document.capacity = 100;
    document.stackPos = 0;
    document.values = (JsonValue *)malloc(document.capacity * sizeof(JsonValue));
    document.stack = (JsonType *)malloc(document.capacity * sizeof(JsonType));
    char * bufferPos = buffer;
    while (*bufferPos != '\0')
    {
        int skipped = json_skipWhitespaces(bufferPos);
        bufferPos += skipped;
        printf("%c", *bufferPos);
        bufferPos++;
    }
    
#if 1
    bufferPos = buffer;
    while (*bufferPos != '\0')
    {
        skip_whitespaces_and_linebreaks(&bufferPos);
        switch (*bufferPos)
        {
            case '{':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_OBJECT;
                val.size = 0;
                json_add(val);
                json_push(val.type);
            }
            break;
            
            case '}':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_OBJECT_CLOSE;
                val.size = 0;
                json_add(val);
                json_pop();
            }
            break;
            
            case '[':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_ARRAY;
                val.size = 0;
                json_add(val);
                json_push(val.type);
            }
            break;
            
            case ']':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_ARRAY_CLOSE;
                val.size = 0;
                json_add(val);
                json_pop();
            }
            break;
            
            case '"':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_STRING;
                val.size = 0;
                bufferPos += json_name(val.name, bufferPos);
                json_add(val);
                json_push(val.type);
            }
            break;
            
            case 'f':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_FALSE;
                val.size = 0;
                json_add(val);
                json_push(val.type);
                advance_to_next_whitespace(&bufferPos);
            }
            break;
            
            case 't':
            {
                bufferPos++;
                JsonValue val;
                val.type = JSON_TRUE;
                val.size = 0;
                json_add(val);
                json_push(val.type);
                advance_to_next_whitespace(&bufferPos);
            }
            break;
            
            default:
            {
                if (*bufferPos >= '0' && *bufferPos <= '9') {
                    char asciiNumber[32];
                    bufferPos += json_number(asciiNumber, bufferPos);
                    JsonValue val;
                    val.type = JSON_NUMBER;
                    val.f_num = atof(asciiNumber);
                    json_add(val);
                    json_push(val.type);
                    advance_to_next_whitespace(&bufferPos);
                }
            }
        }
        bufferPos++;
    }
#endif
    
    print_tokens(&document);
    
    return document;
}

void print_tokens(JsonDocument * doc)
{
    int indent = 0;
    JsonValue * value = doc->values;
    printf("document-size: %d\n\n", doc->size);
    for (int i=0; i<doc->size; ++i)
    {
        switch (value->type)
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
                printf("%s\n", value->name);
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
        }
        value++;
    }
}

#endif

#endif