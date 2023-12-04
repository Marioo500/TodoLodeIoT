# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)
set(requires "")
idf_component_register(SRCS "practica3.c"
                    INCLUDE_DIRS "." )
