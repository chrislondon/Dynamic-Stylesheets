/* All of these include files can be found in the Apache http source
* tree in include/ . If you've got the Apache http server already
* installed, then there will be an include directory either under the
* directory that was specified with --prefix or somewhere in your
* standard include paths. All the functions that you can make use of can be
* found in the include files. */

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "http_request.h"
#include "util_script.h"
#include "http_connection.h"

struct dss_node_list {
	char *key, *val;
	struct dss_node * next;
};

typedef struct dss_node_list dss_node;

/* This module just takes a pointer to the request record as its only
* argument */
static int dss_handler(request_rec *r) {

        /* We decline to handle a request if dss-handler is not the value
         * of r->handler */
        if (strcmp(r->handler, "dss-handler")) {
               return DECLINED;
        }

        /* Open the dss file */
        FILE *f = fopen (r->filename, "r");

        /* If file wasn't opened, it's probably a permission error */
        if (f == NULL) {
        	ap_log_error(APLOG_MARK, APLOG_NOERRNO|APLOG_NOTICE, 0, r->server,
        			"mod_dss: %s", "Unable to open file");
        	return DECLINED;
        }

        /* We set the content type before doing anything else */
        ap_set_content_type(r, "text/css");

        /* If the request is for a header only, and not a request for
         * the whole content, then return OK now. We don't have to do
         * anything else. */
        if (r->header_only) {
                return OK;
        }

        T_fread(f, r);

        fclose (f);

        /* We can either return OK or DECLINED at this point. If we return
        * OK, then no other modules will attempt to process this request */
        return OK;
}


/* Each function our module provides to handle a particular hook is
* specified here. See mod_example.c for more information about this
* step. Suffice to say that we need to list all of our handlers in
* here. */
static void x_register_hooks(apr_pool_t *p) {
        ap_hook_handler(dss_handler, NULL, NULL, APR_HOOK_MIDDLE);
}


/* Module definition for configuration. We list all callback routines
* that we provide the static hooks into our module from the other parts
* of the server. This list tells the server what function will handle a
* particular type or stage of request. If we don't provide a function
* to handle a particular stage / callback, use NULL as a placeholder as
* illustrated below. */
module AP_MODULE_DECLARE_DATA dss_module = {
        STANDARD20_MODULE_STUFF,
        NULL, /* per-directory config creator */
        NULL, /* directory config merger */
        NULL, /* server config creator */
        NULL, /* server config merger */
        NULL, /* command table */
        x_register_hooks, /* other request processing hooks */
};

int T_fread(FILE *input, request_rec *r) {
	long  i, j, k, l, lFileLen;              /* Length of file */
	char *cFile, *cCurrent;        	/* Dynamically allocated buffer (entire file) */
	char *cKey, *cVal;

	dss_node * temp_node, * all_nodes;
	all_nodes = NULL;

	fseek(input, 0L, SEEK_END);  /* Position to end of file */
	lFileLen = ftell(input);     /* Get file length */
	rewind(input);               /* Back to start of file */

	cFile = calloc((lFileLen + 1), sizeof(char));

	if(cFile == NULL ) {
		ap_rprintf(r, "\nInsufficient memory to read file.\n");
		return 0;
	}

	fread(cFile, lFileLen, 1, input); /* Read the entire file into cFile */

	cCurrent = cFile;

	while (*cCurrent) {
		/* look for @ symbols to connotate variable */
		if (cCurrent[0] == '@') {
			/* loop through the rest of the characters to find other delimiters */
			for (i = 1, l = strlen(cCurrent) - 1; i < l; i++) {
				/* If we find a colon that means this is a setter*/
				if (cCurrent[i] == ':') {
					/* store the key */
					cKey = calloc((i + 1), sizeof(char));

					for (j = 1; j < i; j++) {
						cKey[j - 1] = cCurrent[j];
					}

					/* loop through to get the val */
					for (; i < l; i++) {
						if (cCurrent[i] == ';') {
							cVal = calloc((i + 1 - j), sizeof(char));
							for (k = 1; k < (i - j); k++) {
								cVal[k-1] = cCurrent[j + k];
							}

							break;
						}
					}

					/* load key and val into node list */
					temp_node = (dss_node *)malloc(sizeof(dss_node));
					temp_node->key = cKey;
					temp_node->val = cVal;
					temp_node->next = all_nodes;
					all_nodes = temp_node;

					/* move pointer to after variable */
					for (l = i, i = 0; i <= l; i++) {
						*cCurrent++;
					}
					break;
				} else if (cCurrent[i] == ';') {
					/* we found a semi-colon instead of a colon so this is a getter */
					cKey = calloc((i + 1), sizeof(char));

					for (j = 1; j < i; j++) {
						cKey[j - 1] = cCurrent[j];
					}

					temp_node = all_nodes;

					/* loop through all the nodes and compare the keys */
					while(temp_node) {
						/* when we match keys print out the val */
						if (strcmp(temp_node->key, cKey)) {
							ap_rprintf(r, temp_node->val);
							break;
						}

						temp_node = temp_node->next;
					}

					// print out stored val
					for (l = i - 1, i = 0; i < l; i++) {
						*cCurrent++;
					}
					break;
				}
			}

			*cCurrent++;
		} else {
			/* print out non-variable characters */
			ap_rprintf(r, "%c", cCurrent[0]);
			*cCurrent++;
		}
	}

	/* garbage collecting */
	free(cFile);
	free(cKey);
	free(cVal);
	free(temp_node);
	free(all_nodes);

	return 1;
} /* end T_fread() */
