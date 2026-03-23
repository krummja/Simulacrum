import math


def main() -> None:
    width = 1280
    height = 600

    tiles = []
    for i in range(500):
        tiles.append("X")

    index = 0
    for x in range(0, width, 16):
        for y in range(0, height, 16):
            index += 1

            if index >= len(tiles):
                break

            tile = tiles[index]

            print([x, y, tile])



if __name__ == '__main__':
    main()
