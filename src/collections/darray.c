#include <string.h>
#include <time.h>
#include <collections/darray.h>
#include <util/random.h>
#include <assert.h>

DArray DArray_create(size_t element_size, size_t initial_capacity)
{
  DArray array;

  if (!initial_capacity > 0 || !(array = malloc(sizeof(struct DArray)))) return NULL;
  array->capacity = initial_capacity;

  if (!(array->contents = calloc(initial_capacity, sizeof(void *)))) return NULL;

  array->size = 0;
  array->element_size = element_size;
  array->expand_rate = DEFAULT_EXPAND_RATE;

  return array;
}

void DArray_clear(DArray array)
{
  if (array && array->element_size > 0)
  {
    int i = 0;
    for (i = 0; i < array->capacity; i++)
    {
      if (array->contents[i] != NULL)
      {
        void *el = DArray_remove(array, i);
        free(el);
      }
    }
    array->size = 0;
  }
}

static inline int DArray_resize(DArray array, size_t newsize)
{
  if (newsize <= 0) return -1;
  array->capacity = newsize;

  void *contents;
  if (!(contents = realloc(array->contents, array->capacity * sizeof(void *)))) return -1;

  array->contents = contents;

  return 0;
}

int DArray_expand(DArray array)
{
  size_t old_capacity = array->capacity;
  if (DArray_resize(array, array->capacity + array->expand_rate)) return -1;

  memset(array->contents + old_capacity, 0, array->expand_rate * sizeof(void *));
  return 0;
}

int DArray_contract(DArray array)
{
  int new_size = array->size < (int)array->expand_rate ? (int)array->expand_rate : array->size;

  return DArray_resize(array, new_size + 1);
}


void DArray_destroy(DArray array)
{
  if (array)
  {
    if (array->contents) free(array->contents);
    free(array);
  }
}

void DArray_clear_destroy(DArray array)
{
  DArray_clear(array);
  DArray_destroy(array);
}

int DArray_push(DArray array, void *el)
{
  array->contents[array->size] = el;
  array->size++;

  if (DArray_end(array) >= DArray_capacity(array))
  {
    return DArray_expand(array);
  }
  else
  {
    return 0;
  }
}

void *DArray_pop(DArray array)
{
  if (array->size - 1 < 0) return NULL;

  void *el = DArray_remove(array, array->size - 1);
  array->size--;

  if (DArray_end(array) > (int)array->expand_rate && DArray_end(array) % array->expand_rate)
  {
    DArray_contract(array);
  }

  return el;
}

static inline void swap(void **src, void **dst, size_t i, size_t j)
{
  if (src == dst)
  {
    void *tmp;
    tmp = dst[i];
    dst[i] = dst[j];
    dst[j] = tmp;
  }
  else
  {
    dst[i] = src[j];
    dst[j] = src[i];
  }
}

void DArray_remove_all(DArray array, DArray to_remove)
{
  if (!to_remove || !array) return;

  for (size_t i = 0; i < (size_t) to_remove->size; i++)
  {
    void *rem = DArray_get(to_remove, i);
    for (size_t j = 0; j < (size_t) array->size; j++) {
      void *el = DArray_get(array, j);
      if (rem == el) DArray_remove(array, j);
    }
  }
}

void DArray_shuffle(DArray array)
{
  if (!array || !array->size) return;

  size_t i, j;

  for (j = array->size - 1; j > 1; j--)
  {
    i = (random_seq() % j) + 1;
    swap(array->contents, array->contents, i - 1, j - 1);
  }
}


DArray DArray_filter(DArray array, DArray_filter_fn *filter_fn, void* args)
{
  if (DArray_is_empty(array)) return NULL;

  DArray filtered = DArray_create(array->element_size, array->size);

  for (int i = 0; i < DArray_count(array); i++) {
    void *item = DArray_get(array, i);

    if (filter_fn(item, args))
      DArray_push(filtered, item);
  }

  return filtered;
}

int DArray_push_all(DArray dst, DArray src)
{
  if (!dst || !src || DArray_count(src) == 0) return -1;

  assert(dst->element_size == src->element_size);

  for (size_t i = 0; i < (size_t) src->size; i++)
    DArray_push(dst, DArray_get(src, i));

  return 0;
}

/*
void DArray_sort(DArray array, int(*sort_fn)(const void *, const void *))
{
  if (!array) return;
  qsort(array->contents, DArray_count(array), sizeof(void *), sort_fn);
}

void* DArray_bsearch(const void **key, DArray arr,
                         int (*sort_fn)(const void *, const void *))
{
  if (!arr) return NULL;
  return bsearch(key, arr->contents, DArray_count(arr), sizeof(void *),
                 sort_fn);
}

*/
