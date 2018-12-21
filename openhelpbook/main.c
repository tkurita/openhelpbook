#include <stdio.h>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include <getopt.h>

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

int main(int argc, char * const argv[]) {
    
    if (argc <= 1) {
        fprintf(stderr, "No arguments.\n");
        usage();
        return 1;
    }
    
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
                exit(0);
            case 'v':
                showVersion();
                exit(0);
            case '?':
            default	:
                fprintf(stderr, "There is unknown option.\n");
                usage(); 
                exit(-1);
                break;
        }
        optarg	=	NULL;
    }
    
    char *inpath = argv[optind];
    CFBundleRef bundle = NULL;
    CFStringRef bookname= NULL;
    CFStringRef filepath = CFStringCreateWithCString(NULL,inpath,kCFStringEncodingUTF8);
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
