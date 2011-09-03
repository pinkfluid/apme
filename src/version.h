#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

#define APME_VERSION_MAJOR      0
#define APME_VERSION_MINOR      2
#define APME_VERSION_REVISION   1
#define APME_VERSION_NAME       "Nanos"

#define __APM_STR(x)            #x
#define APM_STR(x)              __APM_STR(x)

#define APME_VERSION_STRING  APM_STR(APME_VERSION_MAJOR) "." \
                             APM_STR(APME_VERSION_MINOR) "." \
                             APM_STR(APME_VERSION_REVISION) " "\
                             "(" APME_VERSION_NAME ")"

#endif /* VERSION_H_INCLUDED */
