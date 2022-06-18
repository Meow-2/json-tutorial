#include "leptjson.h"

#include <cassert> /* assert() */
#include <cmath>
#include <cstddef>
#include <cstdlib> /* NULL, strtod() */

#define EXPECT(c, ch)             \
    do {                          \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct
{
    const char* json;
} lept_context;

static void lept_parse_whitespace(lept_context* c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* parse_type,
                              lept_type parse_type_enum)
{
    EXPECT(c, *parse_type);
    parse_type++;
    while (*parse_type != '\0') {
        if (*c->json != *parse_type) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c->json++;
        parse_type++;
    }
    v->type = parse_type_enum;
    return LEPT_PARSE_OK;
}

static bool lept_parse_is_valid_number(lept_context* c)
{
    const char* json = c->json;
    if (*json == '-')
        json++;
    if (*json == '0')
        json++;
    else if (ISDIGIT1TO9(*json)) {
        json++;
        while (ISDIGIT(*json))
            json++;
    }
    else
        return LEPT_PARSE_INVALID_VALUE;
    if (*json == '.') {
        json++;
        while (ISDIGIT(*json))
            json++;
    }
    if (*json == 'E' || *json == 'e') {
        json++;
        if (*json == '+' || *json == '-')
            json++;
        while (ISDIGIT(*json))
            json++;
    }
    if (*json == '\0')
        return true;
    return false;
}
static int lept_parse_number(lept_context* c, lept_value* v)
{
    /* \TODO validate number */
    const char* p = c->json;
    if (*p == '-')
        p++;
    if (*p == '0')
        p++;
    else if (ISDIGIT1TO9(*p)) {
        p++;
        while (ISDIGIT(*p))
            p++;
    }
    else
        return LEPT_PARSE_INVALID_VALUE;
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }
    if (*p == 'E' || *p == 'e') {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }
    if (p == c->json)
        return LEPT_PARSE_INVALID_VALUE;
    v->n = strtod(c->json, NULL);
    if (v->n == HUGE_VAL || v->n == -HUGE_VAL)
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v)
{
    switch (*c->json) {
    case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
    case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
    case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
    default: return lept_parse_number(c, v);
    case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json)
{
    lept_context c;
    int          ret;
    assert(v != NULL);
    c.json  = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret     = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
