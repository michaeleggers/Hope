#ifndef JSON_PARSER_H
#define JSON_PARSER_H


typedef struct JsonValue JsonValue;
typedef struct JsonObject JsonObject;

enum JsonType
{
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_NUMBER,
    JSON_STRING,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL
};

struct JsonStack
{
    JsonType data[1024];
    int pos = 0;
};

static JsonStack json_stack;
static int indent = 0;

struct JsonValue
{
    JsonType type;
    
    union
    {
        JsonObject * object;
        JsonValue * values;
    };
};

struct JsonObject
{
    char name[256];
    JsonValue value;
};

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

void json_push(JsonStack * stack, JsonType type)
{
    for (int i=0; i<indent; ++i)
        printf(" ");
    indent += 2;
    stack->data[stack->pos++] = type;
}

JsonType json_pop(JsonStack * stack, JsonType type)
{
    indent -= 2;
    for (int i=0; i<indent; ++i)
        printf(" ");
    return stack->data[stack->pos--];
}

int json_name(char * in_name, char * buffer)
{
    int length = 0;
    while (*buffer >= 'a' &&
           *buffer <= 'z' ||
           *buffer >= 'A' &&
           *buffer <= 'Z') {
        *in_name++ = *buffer++;
        length++;
    }
    return length;
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
    while (*bufferPos != '\0')
    {
        int skipped = json_skipWhitespaces(bufferPos);
        bufferPos += skipped;
        switch (*bufferPos)
        {
            case '{':
            {
                bufferPos++;
                json_push(&json_stack, JSON_OBJECT);
                printf("JSON_OBJECT\n");
                JsonObject object = {};
                skipped = json_skipWhitespaces(bufferPos);
                bufferPos += skipped;
                skipped = json_name(object.name, bufferPos);
                bufferPos += skipped;
                printf("%s\n", object.name);
            }
            break;
            
            case '}':
            {
                bufferPos++;
                json_pop(&json_stack, JSON_OBJECT);
                printf("JSON_OBJECT\n");
            }
            break;
            
            case '[':
            {
                bufferPos++;
                json_push(&json_stack, JSON_ARRAY);
                printf("JSON_ARRAY\n");
            }
            break;
            
            case ']':
            {
                bufferPos++;
                json_pop(&json_stack, JSON_ARRAY);
                printf("JSON_ARRAY\n");
            }
            break;
        }
        bufferPos++;
    }
#endif
    
    return result;
}

#endif

#endif