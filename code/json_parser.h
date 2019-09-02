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
    JSON_NULL
};

struct JsonValue
{
    JsonType type;
    
    union
    {
        char name[256];
        bool true_or_false;
        int i_num;
        float f_num;
        int array_size;
    };
    
    JsonValue * child;
};

struct JsonRoot
{
    JsonValue * values;
};

struct JsonStack
{
    JsonValue data[1024];
    int pos = 0;
};

static JsonStack json_stack;
static int indent = 0;

#ifdef JSON_PARSER_IMPLEMENTATION

int json_skipWhitespaces(char * buffer)
{
    int skipped = 0;
    while (*buffer == ' ' || *buffer == '\t') { 
        buffer++;
        skipped++; 
    }
    return skipped;
}

void print_indent()
{
    for (int i=0; i<indent; ++i)
        printf(" ");
}

void json_push(JsonStack * stack, JsonValue value)
{
    //print_indent();
    indent += 2;
    stack->data[stack->pos++] = value;
}

JsonValue json_pop(JsonStack * stack)
{
    indent -= 2;
    //print_indent();
    return stack->data[stack->pos--];
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
    while (*buffer == '"') {buffer++; length++;}
    while (*buffer >= 'a' &&
           *buffer <= 'z' ||
           *buffer >= 'A' &&
           *buffer <= 'Z') {
        *out_name++ = *buffer++;
        length++;
    }
    while (*buffer == '"') {buffer++; length++;}
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

JsonValue parse_json(char * buffer)
{
    JsonValue result = {};
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
    int pos = 0;
    while (*bufferPos != '\0')
    {
        pos += json_skipWhitespaces(bufferPos);
        switch (*bufferPos)
        {
            case '{':
            {
                pos++;
                JsonValue value;
                value.type = JSON_OBJECT;
                value.start = pos;
                value.end = -1;
                json_push(&json_stack, value);
                pos += skip_whitespaces_and_linebreaks(&bufferPos + pos);
            }
            break;
            
            case '}':
            {
                pos++;
                //JsonValue * value = &json_pop(&json_stack);
                //value->end = pos;
                pos += skip_whitespaces_and_linebreaks(&bufferPos + pos);
            }
            break;
            
            case '[':
            {
                pos++;
                JsonValue value;
                value.type = JSON_ARRAY;
                value.start = pos;
                value.end = -1;
                json_push(&json_stack, value);
                pos += skip_whitespaces_and_linebreaks(&bufferPos + pos);
            }
            break;
            
            case ']':
            {
                pos++;
                //JsonValue * value = &json_pop(&json_stack);
                //value->end = pos;
                pos += skip_whitespaces_and_linebreaks(&bufferPos + pos);
            }
            break;
            
            case '"':
            {
                char tmp[256];
                int skipped = json_name(tmp, bufferPos + pos);
                pos += skipped;
                bufferPos += skipped;
            }
        }
        bufferPos++;
        pos++;
    }
#endif
    
    for (int i=0; i<1024; ++i)
    {
        JsonValue value = json_stack.data[i];
        switch (value.type)
        {
            case JSON_OBJECT:
            {
                printf("OBJECT\n");
                printf("start: %d ", value.start);
                printf("end: %d\n", value.end);
            }
            break;
            
            case JSON_ARRAY:
            {
                printf("ARRAY\n");
                printf("start: %d ", value.start);
                printf("end: %d\n", value.end);
            }
            break;
        }
    }
    
    return result;
}

#endif

#endif