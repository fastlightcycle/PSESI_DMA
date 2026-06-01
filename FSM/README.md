Graph creation website: https://tifennf.github.io/gratinaph/

Command to generate graphviz PNGs:
```
for file in *.gv; do unflatten -l 3 "$file" | dot -Tpng -o "${file%.gv}.png"; done
```

OR

```
for file in *.gv; do neato -Tpng "$file" > "${file}.png"; done
```
