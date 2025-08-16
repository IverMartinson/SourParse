
# SourParse

World's Best TTF Parser, 2025

### Todo List

- [x] fix memory leaks
- [ ] read composite glyphs' child glyphs into memory as positions (I actualy might make it just be references to glyphs and then just compute the values at runtime, but with a whole ttf file loaded i think its only like 1mb super maximum of memory so it really doesn't matter)
- [ ] add support for other formats
- [ ] add complete TTF support (all the platforms and encodings. Well not the deprecated/obscure ones probably)