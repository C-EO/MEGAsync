#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION 5, 10, 0, 2
#define VER_FILEVERSION_CODE 51000
#define VER_PRODUCTVERSION 5, 10, 0, 2
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR "5.10.0.2\0"

#define VER_BUILD_ID 2

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR "Mega Limited 2025\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

/* SDK commit hash, 7 chars */
#define VER_SDK_ID "b28f39f"

/* Update scrips relying on this value if you move it
Format: 1 item by line, starting from line following the #define
#define VER_CHANGES_NOTES QT_TRANSLATE_NOOP("Preferences",
"- item 1\n"
"- item 2\n\"
 [...]
"- item n\n"
)*/
#define VER_CHANGES_NOTES \
    QT_TRANSLATE_NOOP( \
        "Preferences", \
        "- Introducing a new remote Cloud drive explorer. Browse your Cloud drive in style and " \
        "manage your files and folders in the new sleek and easy-to-use interface.\n" \
        "- We’ve made more improvements to the initial onboarding wizard.\n" \
        "- Systems notifications have been enhanced.\n" \
        "- You can now pin the MEGA app to the Windows taskbar.\n" \
        "- Some third party libraries have been updated.\n" \
        "- Other bugs have been fixed and numerous improvements made.\n")

#endif // VERSION_H
