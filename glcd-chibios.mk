

# List of all the GLCD source files
GLCDSRC =     ${GLCD_DIR}/glcd.c \
              ${GLCD_DIR}/graphics.c \
              ${GLCD_DIR}/graphs.c \
              ${GLCD_DIR}/text.c \
              ${GLCD_DIR}/text_tiny.c \
              $(wildcard ${GLCD_DIR}/controllers/*.c) \
              $(wildcard ${GLCD_DIR}/devices/*.c)

#${GLCD_DIR}/unit_tests.c \

# Required include directories
GLCDINC =     ${GLCD_DIR}/ \
              ${GLCD_DIR}/controllers \
              ${GLCD_DIR}/devices \
              ${GLCD_DIR}/fonts 

