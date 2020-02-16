 find . -name '*Kopie*' | awk '/Kopie/ {print $1}' | xargs -I _ mv "_ Kopie.jpg" "_.jpg"
