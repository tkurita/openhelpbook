#include <stdio.h>
#include <Carbon/Carbon.h>
#include <getopt.h>
#include <sys/param.h> // MAXPATHLEN
#include <stdlib.h> // realpath, perror

#define BUFFER_SIZE MAXPATHLEN+1

enum {
    regiesrHBErr            = 1850,  /*Failed to register HelpBook*/
    NoCFBundleHelpBookName = 1852 /*No CFBundleHelpBookName in Info.plist*/
};

void usage() {
    printf("Usage: openhelpbook [-h] [-v] [filename]\n");
}

void showVersion() {
    printf("openhelpbook 1.0 copyright 2018, Tetsuro KURITA\n");
}


void display_error_message(OSStatus err , char* filename) {
    switch (err) {
        case -50: /* paramErr : error in user parameter list.
                   defoined in MacErrors.h */
            fprintf(stderr, "Failed to register HelpBook for \n\"%s\".\n", filename);
            break;
        case NoCFBundleHelpBookName:
            fprintf(stderr,"No CFBundleHelpBookName in Info.plist of \"%s\".\n", filename);
            break;
        default:
            break;
    }
}

void safeRelease(CFTypeRef theObj)
{
    if (theObj != NULL) {
        CFRelease(theObj);
    }
}

OSStatus open_helpbook(char *inpath) {
    CFBundleRef bundle = NULL;
    CFStringRef bookname= NULL;
    CFStringRef filepath = CFStringCreateWithCString(NULL, inpath, kCFStringEncodingUTF8);
    CFURLRef bundle_url = CFURLCreateWithFileSystemPath(NULL,filepath, kCFURLPOSIXPathStyle, true);
    OSStatus status = AHRegisterHelpBookWithURL(bundle_url);
    if (noErr != status) {
        display_error_message(status, inpath);
        goto bail;
    }
    bundle = CFBundleCreate(NULL, bundle_url);
    bookname = CFBundleGetValueForInfoDictionaryKey(bundle, CFSTR("CFBundleHelpBookName"));
    if (!bookname) {
        display_error_message(NoCFBundleHelpBookName, inpath);
        goto bail;
    }
    status = AHGotoPage(bookname, NULL, NULL);
    if (noErr != status) {
        display_error_message(status, inpath);
    }
bail:
    safeRelease(filepath);
    safeRelease(bundle_url);
    safeRelease(bundle);
    safeRelease(bookname);
    return status;
}

int main(int argc, char * const argv[]) {
    
    static struct option long_options[] = {
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {0, 0}
    };
    
    int c;
    int option_index = 0;
    while(1){
        c = getopt_long(argc, argv, "vh",long_options, &option_index);
        if (c == -1)
            break;
        
        switch(c){
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'v':
                showVersion();
                exit(EXIT_SUCCESS);
            case '?':
            default	:
                fprintf(stderr, "There is unknown option.\n");
                usage(); 
                exit(EXIT_FAILURE);
                break;
        }
        optarg	=	NULL;
    }
    OSStatus status = EXIT_SUCCESS;
    char buf[BUFFER_SIZE];
    if (optind == argc) { // No arguments
        if (getwd(buf) == NULL) {
            perror("getwd error");
            exit(EXIT_FAILURE);
        }
        status = open_helpbook(buf);
    } else {
        for (short i = optind; i < argc; i++) {
            char *inpath = argv[optind];
            char *res = realpath(inpath, buf);
            if (! res) {
                perror("realpath error");
                exit(EXIT_FAILURE);
            }
            status = open_helpbook(buf);
            if (noErr != status) exit(status);
        }
    }
    return status;
}
