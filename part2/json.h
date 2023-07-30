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

bool json_is_value(enum JsonType type)
{
  return type >= JsonObject && type <= JsonString;
}

char next_char(FILE * file)
{
  char c;
  size_t result = fread(&c, 1, 1, file);
  assert(result == 1);
  return c;
}

void unread_char(FILE * file)
{
  fseek(file, -1, SEEK_CUR);
}

Json * json_parse_file(FILE * file)
{
  Json * ret = calloc(sizeof(Json), 1);
  char c;
  do
  {
    c = next_char(file);
  }
  while (c == ' ' || c == '\n');
  if (c == '"')
  {
    ret->type = JsonString;
    ret->string = malloc(256);
    char * p = ret->string;
    while (true)
    {
      c = next_char(file);
      if (c == '"') { break; }
      // WARNING: Buffer overflow below if strings are too long.
      *p++ = c;
    }
    *p = 0;
  }
  else if (c == '-' || (c >= '0' && c <= '9'))
  {
    ret->type = JsonNumber;
    // WARNING: buffer overflow below if floats are too long
    char float_string[256];
    char * p = float_string;
    float_string[0] = c;
    while (true)
    {
      c = next_char(file);
      if (c == '-' || (c >= '0' && c <= '9') || c == '.')
      {
        *p++ = c;
      }
      else
      {
        unread_char(file);
        break;
      }
    }
    *p = 0;
    ret->number = strtod(float_string, NULL);
  }
  else if (c == '{')
  {
    ret->type = JsonObject;
    Json ** tip = &ret->first;
    while (true)
    {
      Json * first = json_parse_file(file);
      if (first->type == JsonObjectEnd)
      {
        free(first);
        break;
      }
      *tip = first;

      Json * colon = json_parse_file(file);
      assert(colon->type == JsonColon);
      free(colon);

      Json * value = json_parse_file(file);
      assert(json_is_value(value->type));
      first->next = value;

      tip = &value->next;

      Json * last = json_parse_file(file);
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
  }
  else if (c == '[')
  {
    ret->type = JsonArray;
    Json ** tip = &ret->first;
    while (true)
    {
      Json * element = json_parse_file(file);
      if (element->type == JsonArrayEnd)
      {
        free(element);
        break;
      }
      assert(json_is_value(element->type));
      *tip = element;
      tip = &element->next;

      Json * last = json_parse_file(file);
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
  return ret;
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
