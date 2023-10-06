enum JsonType
{
  JsonTypeNone,

  // Actual values
  JsonObject,  // just like an array, but every other entry is a name
  JsonArray,
  JsonNumber,
  JsonString,

  // Tokens, not part of the final structure returned.
  JsonComma,
  JsonColon,
  JsonObjectEnd,
  JsonArrayEnd,
};

typedef struct Json
{
  enum JsonType type;
  union {
    struct Json * first;
    float number;
    char * string;
  };
  struct Json * next;
} Json;

typedef struct JsonInputBuffer
{
  size_t index;
  size_t size;
  char * data;
} JsonInputBuffer;

bool json_is_value(enum JsonType type)
{
  return type >= JsonObject && type <= JsonString;
}

char next_char(JsonInputBuffer * buf)
{
  if (buf->index >= buf->size) { buf->index = buf->size - 1; }
  return buf->data[buf->index++];
}

void unread_char(JsonInputBuffer * buf)
{
  if (buf->index) { buf->index--; }
}

Json * json_parse_core(JsonInputBuffer * buf)
{
  profile_block("json_parse_core");
  Json * ret = calloc(sizeof(Json), 1);
  char c;
  profile_block("jpc - skip spaces");
  do
  {
    c = next_char(buf);
  }
  while (c == ' ' || c == '\n');
  profile_block_done();
  if (c == '"')
  {
    profile_block("jpc - string");
    ret->type = JsonString;
    ret->string = malloc(256);
    char * p = ret->string;
    while (true)
    {
      c = next_char(buf);
      if (c == '"') { break; }
      // WARNING: Buffer overflow below if strings are too long.
      *p++ = c;
    }
    *p = 0;
    profile_block_done();
  }
  else if (c == '-' || (c >= '0' && c <= '9'))
  {
    profile_block("jpc - number");
    ret->type = JsonNumber;
    // WARNING: buffer overflow below if floats are too long
    char float_string[256];
    char * p = float_string;
    float_string[0] = c;
    while (true)
    {
      c = next_char(buf);
      if (c == '-' || (c >= '0' && c <= '9') || c == '.' || c == 'e')
      {
        *p++ = c;
      }
      else
      {
        unread_char(buf);
        break;
      }
    }
    *p = 0;
    profile_block("jpc - strtod");
    ret->number = strtod(float_string, NULL);
    profile_block_done();
    profile_block_done();
  }
  else if (c == '{')
  {
    profile_block("jpc - object");
    ret->type = JsonObject;
    Json ** tip = &ret->first;
    while (true)
    {
      Json * first = json_parse_core(buf);
      if (first->type == JsonObjectEnd)
      {
        free(first);
        break;
      }
      *tip = first;

      Json * colon = json_parse_core(buf);
      assert(colon->type == JsonColon);
      free(colon);

      Json * value = json_parse_core(buf);
      assert(json_is_value(value->type));
      first->next = value;

      tip = &value->next;

      Json * last = json_parse_core(buf);
      if (last->type == JsonObjectEnd)
      {
        free(last);
        break;
      }
      else if (last->type == JsonComma)
      {
        free(last);
      }
      else
      {
        assert(0);
      }
    }
    profile_block_done();
  }
  else if (c == '[')
  {
    profile_block("jpc - array");
    ret->type = JsonArray;
    Json ** tip = &ret->first;
    while (true)
    {
      Json * element = json_parse_core(buf);
      if (element->type == JsonArrayEnd)
      {
        free(element);
        break;
      }
      assert(json_is_value(element->type));
      *tip = element;
      tip = &element->next;

      Json * last = json_parse_core(buf);
      if (last->type == JsonArrayEnd)
      {
        free(last);
        break;
      }
      else if (last->type == JsonComma)
      {
        free(last);
      }
      else
      {
        assert(0);
      }
    }
    profile_block_done();
  }
  else if (c == ':')
  {
    ret->type = JsonColon;
  }
  else if (c == ',')
  {
    ret->type = JsonComma;
  }
  else if (c == '}')
  {
    ret->type = JsonObjectEnd;
  }
  else if (c == ']')
  {
    ret->type = JsonArrayEnd;
  }
  else
  {
    fprintf(stderr, "Unrecognized starting char: %c\n", c);
    assert(0);
  }
  profile_block_done();
  return ret;
}

Json * json_parse_file(FILE * file)
{
  profile_block("json_parse_file");

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char * data = malloc(file_size);

  profile_block("jpf - fread");
  size_t bytes_read = fread(data, 1, file_size, file);
  profile_record_bytes(bytes_read);
  profile_block_done();

  JsonInputBuffer buf = { .size = bytes_read, .data = data };
  Json * r = json_parse_core(&buf);
  free(data);
  profile_block_done();
  return r;
}

Json * json_object_lookup(Json * obj, const char * name)
{
  assert(obj->type == JsonObject);
  for (Json * entry = obj->first; entry; entry = entry->next->next)
  {
    assert(entry->type == JsonString);
    if (0 == strcmp(entry->string, name)) { return entry->next; }
  }
  return NULL;
}
