# Modifies and runs the script to build the protobuf files for use in this library.
# Since this is a library, the normal script itself won't work (or at least I haven't figured out how)
import os
Import("env")

libdeps_dir = env.subst(os.path.join("$PROJECT_LIBDEPS_DIR", "$PIOENV"))
nanopb_root = os.path.join(libdeps_dir, "Nanopb")
pb_generator = os.path.join(nanopb_root, "generator", "platformio_generator.py")

if not os.path.isfile(pb_generator):
    raise ModuleNotFoundError("It seems like the nanopb library is not installed!")

with open(pb_generator) as original_script:
    content = original_script.read()
    # Replace the original definition for nanopb_protos with ours and comment out the old one.
    # I don't like doing this, but it seems like there is no better way...
    # If this ever breaks, I'm sorry :(
    content = content.replace("\nnanopb_protos =", "\nnanopb_protos = '+<../../proto/*.proto>' #")
    # Change the workign directory, otherwise the original script won't work
    os.chdir(os.path.join(nanopb_root, "generator"))
    # Execute the modified content
    exec(content)