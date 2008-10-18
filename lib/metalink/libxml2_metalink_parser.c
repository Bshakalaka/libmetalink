/* <!-- copyright */
/*
 * libmetalink
 *
 * Copyright (c) 2008 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/* copyright --> */
#include "metalink_parser.h"
#include "metalink_config.h"

#include <string.h>

#include <libxml/parser.h>

#include "metalink_pstm.h"
#include "metalink_pstate.h"
#include "metalink_error.h"
#include "metalink_parser_common.h"
#include "session_data.h"
#include "stack.h"
#include "string_buffer.h"

static void start_element_handler(void* user_data,
				  const xmlChar* name,
				  const xmlChar** attrs)
{
  session_data_t* session_data = (session_data_t*)user_data;
  string_buffer_t* str_buf = new_string_buffer(128);

  /* TODO evaluate return value of stack_push; non-zero value is error. */
  stack_push(session_data->characters_stack, str_buf);

  session_data->stm->state->start_fun(session_data->stm,
				      (const char*)name,
				      (const char**)attrs);
}

static void end_element_handler(void* user_data, const xmlChar* name)
{
  session_data_t* session_data = (session_data_t*)user_data;
  string_buffer_t* str_buf = stack_pop(session_data->characters_stack);
  
  session_data->stm->state->end_fun(session_data->stm,
				    (const char*)name,
				    string_buffer_str(str_buf));

  delete_string_buffer(str_buf);	     
}

static void characters_handler(void* user_data, const xmlChar* chars,
			       int length)
{
  session_data_t* session_data = (session_data_t*)user_data;
  string_buffer_t* str_buf = stack_top(session_data->characters_stack);

  string_buffer_append(str_buf, (const char*)chars, length);
}

static xmlSAXHandler mySAXHandler = {
  0, /* internalSubsetSAXFunc */
  0, /* isStandaloneSAXFunc */
  0, /* hasInternalSubsetSAXFunc */
  0, /* hasExternalSubsetSAXFunc */
  0, /* resolveEntitySAXFunc */
  0, /* getEntitySAXFunc */
  0, /* entityDeclSAXFunc */
  0, /* notationDeclSAXFunc */
  0, /* attributeDeclSAXFunc */
  0, /* elementDeclSAXFunc */
  0, /*   unparsedEntityDeclSAXFunc */
  0, /*   setDocumentLocatorSAXFunc */
  0, /*   startDocumentSAXFunc */
  0, /*   endDocumentSAXFunc */
  &start_element_handler, /*   startElementSAXFunc */
  &end_element_handler, /*   endElementSAXFunc */
  0, /*   referenceSAXFunc */
  &characters_handler, /*   charactersSAXFunc */
  0, /*   ignorableWhitespaceSAXFunc */
  0, /*   processingInstructionSAXFunc */
  0, /*   commentSAXFunc */
  0, /*   warningSAXFunc */
  0, /*   errorSAXFunc */
  0, /*   fatalErrorSAXFunc */
  0, /*   getParameterEntitySAXFunc */
  0, /*   cdataBlockSAXFunc */
  0, /*   externalSubsetSAXFunc */
  0, /*   unsigned int  initialized */
  0, /*   void *        _private */
  0, /*   startElementNsSAX2Func */
  0, /*   endElementNsSAX2Func */
  0, /*   xmlStructuredErrorFunc */
};

metalink_error_t
METALINK_PUBLIC
metalink_parse_file(const char* filename, metalink_t** res)
{
  session_data_t* session_data;
  metalink_error_t r,
  		   retval;

  session_data = new_session_data();
  
  r = xmlSAXUserParseFile(&mySAXHandler, session_data, filename);

  retval = metalink_handle_parse_result(res, session_data, r);

  delete_session_data(session_data);

  return retval;
}

metalink_error_t
METALINK_PUBLIC
metalink_parse_fp(FILE* docfp, metalink_t** res)
{
  session_data_t* session_data;
  metalink_error_t r = 0,
  		   retval;
  size_t num_read;
  char buff[BUFSIZ];

  xmlParserCtxtPtr ctxt;

  session_data = new_session_data();
  
  num_read = fread(buff, 1, 4, docfp);
  ctxt = xmlCreatePushParserCtxt(&mySAXHandler, session_data, buff, num_read, NULL);
  if(ctxt == NULL)
	  r = METALINK_ERR_PARSER_ERROR;

  while(!feof(docfp) && !r) {
    num_read = fread(buff, 1, BUFSIZ, docfp);
    if(num_read < 0 || xmlParseChunk(ctxt, buff, num_read, 0))
      r = METALINK_ERR_PARSER_ERROR;
  }
  xmlParseChunk(ctxt, buff, 0, 1);
  xmlFreeParserCtxt(ctxt);

  retval = metalink_handle_parse_result(res, session_data, r);

  delete_session_data(session_data);

  return retval;
}

metalink_error_t
METALINK_PUBLIC
metalink_parse_memory(const char* buf, size_t len, metalink_t** res)
{
  session_data_t* session_data;
  metalink_error_t r,
		   retval;

  session_data = new_session_data();

  r = xmlSAXUserParseMemory(&mySAXHandler, session_data, buf, len);

  retval = metalink_handle_parse_result(res, session_data, r);

  delete_session_data(session_data);

  return retval;
}

struct _metalink_parser_context
{
  session_data_t* session_data;
  xmlParserCtxtPtr parser;
  metalink_t* res;
};

metalink_parser_context_t
METALINK_PUBLIC
* new_metalink_parser_context()
{
  metalink_parser_context_t* ctx;
  ctx = malloc(sizeof(metalink_parser_context_t));
  if(ctx == NULL) {
    return NULL;
  }
  memset(ctx, 0, sizeof(metalink_parser_context_t));

  ctx->session_data = new_session_data();
  if(ctx->session_data == NULL) {
    delete_metalink_parser_context(ctx);
    return NULL;
  }
  return ctx;
}

void
METALINK_PUBLIC
delete_metalink_parser_context(metalink_parser_context_t* ctx)
{
  if(ctx == NULL) {
    return;
  }
  delete_session_data(ctx->session_data);
  xmlFreeParserCtxt(ctx->parser);
  free(ctx);
}

metalink_error_t
METALINK_PUBLIC
metalink_parse_update_internal(metalink_parser_context_t* ctx,
			       const char* buf, size_t len, int terminate)
{
  metalink_error_t r;

  if(ctx->parser == NULL) {
    int inilen = 4 < len ? 4 : len;
    ctx->parser = xmlCreatePushParserCtxt(&mySAXHandler, ctx->session_data,
					  buf, inilen, NULL);
    if(ctx->parser == NULL) {
      r = METALINK_ERR_PARSER_ERROR;
    } else {
      r = xmlParseChunk(ctx->parser, buf+inilen, len-inilen, terminate);
    }
  } else {
    r = xmlParseChunk(ctx->parser, buf, len, terminate);
  }
  return r;
}

metalink_error_t
METALINK_PUBLIC
metalink_parse_update(metalink_parser_context_t* ctx,
		      const char* buf, size_t len)
{
  metalink_error_t r;
  r = metalink_parse_update_internal(ctx, buf, len, 0);
  if(r == 0) {
    r = metalink_pctrl_get_error(ctx->session_data->stm->ctrl);
  }
  return r;
}

metalink_error_t
METALINK_PUBLIC
metalink_parse_final(metalink_parser_context_t* ctx,
		     const char* buf, size_t len, metalink_t** res)
{
  metalink_error_t r,
		   retval;

  r = metalink_parse_update_internal(ctx, buf, len, 1);
  if(r == 0) {
    r = metalink_pctrl_get_error(ctx->session_data->stm->ctrl);
  }

  retval = metalink_handle_parse_result(res, ctx->session_data, r);

  delete_metalink_parser_context(ctx);

  return retval; 
}
