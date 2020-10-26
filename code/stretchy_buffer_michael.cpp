//// NOT A FULL IMPLEMENTATION! THIS WAS A TRIAL AND ERROR THINGY!!!
////////////////////////////////////////////////////////////////////

struct HopeVector
{
    void * ptrToArray;
};

#define hope_vector(type, object) type * object; \
object = (type *)malloc(sizeof(type)*3 + 2*sizeof(int)); \
*(int *)object = 3; \
((int *)object)[1] = 0; \
object = (type *)&((int *)object)[2];

#define hope_vector_size(object) \
((int *)object)[-2];

struct Foo
{
    int a, b;
};

#define hope_vector_push_back(object, item) \
hope_vector_create( *((void **)&object)
/*
if (object == 0) { \
    object = (Foo *)malloc(sizeof(item)*3 + 2*sizeof(int)); \
    *(int *)object = 3; \
    ((int *)object)[1] = 0; \
} \
object = (Foo *)&((int *)object)[2]; \
object[((int *)object)[-1]] = item; \
((int *)object)[-1] += 1;
*/

//if (! ((int *)object)[-2] - ((int *)object)[-1] ) \
//object = (int *)realloc( (void *)object, ((int *)object)[-2] * 2 * sizeof(object[0]) ); \
//object[((int *)object)[-1]] = item; \
//((int *)object)[-1] += 1;

struct Bar
{
    char c;
};

void push (void ** foothing, int sizeOfItem, void * item)
{
#if 1
    void * cpyLocation = 0;
    if (0 == *foothing)
    {
        *foothing = malloc(10*sizeOfItem * 2*sizeof(int));
        *((int *)(*foothing)) = 10;
        ((int *)*foothing)[1] = 0;
        *foothing = &((int *)*foothing)[2];
        cpyLocation = *foothing;
    }
    else
    {
        int currentPos = ((int *)*foothing)[-1];
        cpyLocation = (char *)*foothing + currentPos*sizeOfItem;
    }
    ((int *)*foothing)[-1] += 1;
    hope_memcpy(cpyLocation, item, sizeOfItem);
#endif
}

#define push_macro(array, item) push((void **)&array, sizeof(*array), &item);

Foo aFunction()
{
    return {666,666};
}

typedef int (*someFunctionPtr)(void);

int lol(void)
{
    printf("L OOOOOO L\n");
    return 0;
}


// USAGE CODE

#if 0
Foo fooItem = {1,2};
Foo fooItem2 = {999, 888};
Foo fooItem3 = {321, 123};
Foo * myFoo = 0;
push_macro(myFoo, fooItem);
push_macro(myFoo, fooItem2);
push_macro(myFoo, fooItem3);
push_macro(myFoo, aFunction());
Foo getFooItem = myFoo[0];

Foo * anItem = (Foo *)malloc(sizeof(Foo));
anItem->a = 777;
anItem->b = 89;
Foo ** array = 0;
push_macro(array, anItem);
push_macro(array, anItem);
//push((void**)&myFoo, sizeof(fooItem), (void *)&fooItem);
//push((void**)&myFoo, sizeof(fooItem2), (void *)&fooItem2);

// This won't work in my implementation
someFunctionPtr * funcArray1;
push_macro(funcArray1, lol);

someFunctionPtr * funcArray = 0;
sb_push(funcArray, lol);
someFunctionPtr lolFunc = funcArray[0];
lolFunc();
#endif

#if 0    
std::vector<Foo> fooVec;
fooVec.push_back(someObj);
fooVec[2];

void * thing = malloc(sizeof(int)*10);
((int *)thing)[0] = 999;
((int *)thing)[1] = 777;
*(Bar *)&((int *)thing)[2] = {'a'};

Foo * foo = 0;
//int fooSize = hope_vector_size(foo);
Foo item = {42, 33};
Foo item2 = {66, 99};
//foo[0] = item;
//hope_vector_push_back(foo, item);
//hope_vector_push_back(foo, item);
//hope_vector_push_back(foo, item);
//hope_vector_push_back(foo, item2);
#endif

#if 0
hope_vector(Bar, bar);
int barSize = hope_vector_size(bar);
Bar barItem = {'Q'};
hope_vector_push_back(bar, barItem);
hope_vector_push_back(bar, {'X'});
bar[2] = {'Y'};

hope_vector_push_back(foo, item);
hope_vector_push_back(foo, item);


char a = 'a';
char b = 'b';
char c = 'c';
hope_vector(char, cfoo);
hope_vector_push_back(cfoo, a);
hope_vector_push_back(cfoo, b);
hope_vector_push_back(cfoo, c);
#endif
