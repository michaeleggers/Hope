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
        int i_num;
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


#ifdef JSON_PARSER_IMPLEMENTATION

static int indent = 0;
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

void print_indent()
{
    for (int i=0; i<indent; ++i)
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
        }
        bufferPos++;
    }
#endif
    
    return document;
}

#endif

#endif