// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/****************************************************************************
						 Microsoft RPC 
         
                       umarsh Example

    FILE:       umarshc.c

    USAGE:      umarshc  -n network_address
                       -p protocol_sequence
                       -e endpoint
                       -a server principal name
                       -o options
                       -c count of elements in linked list
                       -v value of first element in linked list
                       -d delta between values in linked list

    PURPOSE:    Client side of RPC distributed application.
                This sample demonstrates the represent_as example.
                A char string is sent over the network as a unicode string.

    RELATED:    umarshs.c - server main
                umarshp.c - remote procedures
                umarshcu.c - client utility procedures

    FUNCTIONS:  main() - bind to server and call remote procedure

    COMMENTS:   This sample program generates a client and server can share
                an interface, but one side can use a different representation
                than the other.

                The client side in this example does all operations using
                character strings, and the server side does all operations
                using UNICODE strings.  Two procedures are provided, one
                defined with ASCII strings, one with UNICODE strings.
                The wire format reflects these definitions, yet the client
                and server see pure ASCII and pure UNICODE respectively.

                The [user_marshal] attribute (used in the client and server
                side acf files) requires the four user-supplied functions
                whose names start with the name of the transmitted type
                (in the client side's case: WCHAR_STRING)

                The [in, out] attributes applied to remote procedure
                parameters require the two user-supplied functions
                midl_user_allocate and midl_user_free.

                The other functions are utilities that are used to
                build or display the data structures.


****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "umarshc.h"    // client's side header file generated by MIDL compiler
#include "spn.h"

#define PURPOSE \
"This Microsoft RPC sample program demonstrates\n\
the use of the [represent_as] attribute. For more information\n\
about the attributes and the RPC API functions, see the\n\
RPC programming guide and reference.\n\n"

#define MAX_ELEMENTS 50
#define BUFFER_SIZE 100

void Usage(char * pszProgramName)
{
    fprintf_s(stderr, "%s", PURPOSE);
    fprintf_s(stderr, "Usage:  %s\n", pszProgramName);
    fprintf_s(stderr, " -p protocol_sequence\n");
    fprintf_s(stderr, " -n network_address\n");
    fprintf_s(stderr, " -e endpoint\n");
    fprintf_s(stderr, " -a server principal name\n");	
    fprintf_s(stderr, " -o options\n");
    fprintf_s(stderr, " -c count_of_elements\n");
    fprintf_s(stderr, " -v value\n");
    fprintf_s(stderr, " -d delta\n");
    exit(1);
}

void __cdecl main(int argc, char **argv)
{
    RPC_STATUS status;
    unsigned char * pszUuid             = NULL;
    unsigned char * pszProtocolSequence = "ncacn_ip_tcp";
    unsigned char * pszNetworkAddress   = NULL;
    unsigned char * pszEndpoint         = "8765";
    unsigned char * pszSpn              = NULL;	
    unsigned char * pszOptions          = NULL;
    unsigned char * pszStringBinding    = NULL;
	RPC_SECURITY_QOS SecQos;
    int i;
    int cElements = 10;
    short sValue = 100;
    short sDelta = 10;
    char    FirstBuffer[BUFFER_SIZE];

    /* allow the user to override settings with command line switches */
    for (i = 1; i < argc; i++) {
        if ((*argv[i] == '-') || (*argv[i] == '/')) {
            switch (tolower(*(argv[i]+1))) {
            case 'p':  // protocol sequence
                pszProtocolSequence = argv[++i];
                break;
            case 'n':  // network address
                pszNetworkAddress = argv[++i];
                break;
            case 'e':
                pszEndpoint = argv[++i];
                break;
            case 'a': 
                pszSpn = argv[++i];
                break;
            case 'o':
                pszOptions = argv[++i];
                break;
            case 'c':
                cElements = atoi(argv[++i]);
                if (cElements > MAX_ELEMENTS)
                    cElements = MAX_ELEMENTS;
                break;
            case 'v':
                sValue = (short)atoi(argv[++i]);
                break;
            case 'd':
                sDelta = (short)atoi(argv[++i]);
                break;
            case 'h':
            case '?':
            default:
                Usage(argv[0]);
            }
        }
        else
            Usage(argv[0]);
    }

    /* Use a convenience function to concatenate the elements of the string */
    /* binding into the syntax needed by RpcBindingFromStringBinding.       */
    status = RpcStringBindingCompose(pszUuid,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pszStringBinding);
    printf_s("RpcStringBindingCompose returned 0x%x\n", status);
    printf_s("pszStringBinding = %s\n", pszStringBinding);
    if (status) {
        exit(status);
    }

    /* Set the binding handle that will be used to bind to the server. */
    status = RpcBindingFromStringBinding(pszStringBinding,
                                         &humarsh);
    printf_s("RpcBindingFromStringBinding returned 0x%x\n", status);
    if (status) {
        exit(status);
    }

    /* User did not specify spn, construct one */
    if (pszSpn == NULL) {
        MakeSpn(&pszSpn);
    }

    /* Set the quality of service on the binding handle */
    SecQos.Version = RPC_C_SECURITY_QOS_VERSION_1;
    SecQos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    SecQos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    SecQos.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;

    /* Set the security provider on binding handle */
    status = RpcBindingSetAuthInfoEx(humarsh,
                                     pszSpn,
                                     RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                     RPC_C_AUTHN_GSS_NEGOTIATE,
                                     NULL,
                                     RPC_C_AUTHZ_NONE,
                                     &SecQos);
	
    printf_s("RpcBindingSetAuthInfoEx returned 0x%x\n", status);
    if (status) {
        exit(status);
    }	
		
    RpcTryExcept {
        // initialize a string to be shipped
        strcpy_s(FirstBuffer,BUFFER_SIZE,"This ASCII string is sent from the client to the server as UNICODE");

        printf_s("\nThe Client side is about to send:\n\"%s\"\n\n", FirstBuffer );

        printf_s("Calling the remote procedure 'ModifyMyWString'\n");
        // note that the client sees the interface using ascii, not unicode
        ModifyMyWString(humarsh, &FirstBuffer );
        printf_s("The Client side got back:\n\"%s\"\n\n", FirstBuffer );


        // initialize a string to be shipped
        strcpy_s(FirstBuffer,BUFFER_SIZE,"And this ASCII string is sent from the client to the server as ASCII");

        printf_s("The Client side is about to send:\n\"%s\"\n\n", FirstBuffer );
        printf_s("Calling the remote procedure 'ModifyMyString'\n");
        // note that the client sees the interface using ascii
        ModifyMyString(humarsh, &FirstBuffer );
        printf_s("The Client side got back:\n\"%s\"\n\n", FirstBuffer );

        printf_s("Calling the remote procedure 'Shutdown'\n");
        Shutdown(humarsh);  // shut down the server side
    }
    RpcExcept(( ( (RpcExceptionCode() != STATUS_ACCESS_VIOLATION) &&
                   (RpcExceptionCode() != STATUS_DATATYPE_MISALIGNMENT) &&
                   (RpcExceptionCode() != STATUS_PRIVILEGED_INSTRUCTION) &&
                   (RpcExceptionCode() != STATUS_BREAKPOINT) &&
                   (RpcExceptionCode() != STATUS_STACK_OVERFLOW) &&
                   (RpcExceptionCode() != STATUS_IN_PAGE_ERROR) &&
                   (RpcExceptionCode() != STATUS_GUARD_PAGE_VIOLATION)
                    )
                    ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )) {
        printf_s("Runtime reported exception %ld\n", RpcExceptionCode() );

        
	}
    RpcEndExcept

    /* The calls to the remote procedures are complete.            */
    /* Free the string and the binding handle using RPC API calls. */
    status = RpcStringFree(&pszStringBinding);
    printf_s("RpcStringFree returned 0x%x\n", status);
    if (status) {
        exit(status);
    }

    status = RpcBindingFree(&humarsh);
    printf_s("RpcBindingFree returned 0x%x\n", status);
    if (status) {
        exit(status);
    }

    exit(0);

}  // end main()

/* end file umarshc.c */
