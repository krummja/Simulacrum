from pathlib import Path
import re


HERE = Path(__file__)
SCRIPTS_DIR = HERE.parent
ROOT_DIR = SCRIPTS_DIR.parent
SRC_DIR = Path(ROOT_DIR, "src")


def pascal_to_spaced(string_val: str) -> str:
    pattern = re.compile(
        r"""
        (?<=[a-z])(?=[A-Z0-9]) |    # positive lookbehind for lower
                                    # positive lookahead for upper or digit
                                    # someThing -> some Thing; word3D -> word 3D
        (?<=[A-Z])(?=[A-Z][a-z]) |  # positive lookbehind for upper
                                    # positive lookahead for upper followed by lower
                                    # HTMLParser -> HTML Parser
        (?<=[0-9])(?=[A-Za-z])      # positive lookbehind for digit
                                    # positive lookahead for letter
                                    # 123Word -> 123 Word
        """,
        flags=re.X,
    )
    return re.sub(pattern, " ", string_val)


def spaced_to_snake(string_val: str) -> str:
    return "_".join(string_val.split(" ")).lower()


def snake_to_pascal(name: str) -> str:
    return "".join(x.capitalize() for x in name.lower().split("_"))


def pascal_to_snake(string_val: str) -> str:
    return spaced_to_snake(pascal_to_spaced(string_val))


def get_extension(filename: str) -> str:
    return filename.split(".")[-1]


def replace_symbol(path: Path, file: Path) -> None:
    extension = get_extension(file.name)
    if extension != "cpp":
        return

    stem = file.stem
    target = pascal_to_snake(stem)
    classname = snake_to_pascal(target)

    include_expr = re.compile(r'^(#include\s")([a-z_]*)(.hpp")')

    with open(path, "r") as source:
        lines = source.readlines()

    with open(path, "w") as writeable_source:
        for line in lines:
            result = include_expr.match(line)
            if result is not None:
                result_grps = result.groups()
                before_part = result_grps[0]
                result = result_grps[1]
                after_part = result_grps[-1]
                modified = f"{before_part}{snake_to_pascal(result)}{after_part}\n"
                print([line, modified])
            # line = line.replace(target, classname)
            # _ = writeable_source.write(line)


def main() -> None:
    for _, file in enumerate(SRC_DIR.iterdir()):
        stem = file.stem
        classname = snake_to_pascal(stem)
        # replace_symbol(file, file)

        extension = get_extension(file.name)
        new_file = Path(SRC_DIR, f"{classname}.{extension}")
        _ = file.rename(new_file)


if __name__ == "__main__":
    main()
