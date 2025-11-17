#!/bin/bash

cd data

for file in *.tsp; do
    if [ ! -f "$file" ]; then
        continue
    fi

    type=$(grep "EDGE_WEIGHT_TYPE" "$file" 2>/dev/null | sed 's/.*: *//' | tr -d ' \r')

    if [ "$type" = "ATT" ]; then
        mv "$file" ../uncompatibleData/ATT/
        echo "Moved $file to ATT"
    elif [ "$type" = "GEO" ]; then
        mv "$file" ../uncompatibleData/GEO/
        echo "Moved $file to GEO"
    elif [ "$type" = "EXPLICIT" ]; then
        mv "$file" ../uncompatibleData/EXPLICIT/
        echo "Moved $file to EXPLICIT"
    elif [ "$type" = "CEIL_2D" ]; then
        mv "$file" ../uncompatibleData/OTHER/
        echo "Moved $file to OTHER (CEIL_2D)"
    fi
done

echo ""
echo "Summary:"
echo "EUC_2D files (compatible) remaining in data/:"
ls -1 *.tsp 2>/dev/null | wc -l
echo ""
echo "Files moved to uncompatibleData:"
echo "  ATT: $(ls -1 ../uncompatibleData/ATT/*.tsp 2>/dev/null | wc -l)"
echo "  GEO: $(ls -1 ../uncompatibleData/GEO/*.tsp 2>/dev/null | wc -l)"
echo "  EXPLICIT: $(ls -1 ../uncompatibleData/EXPLICIT/*.tsp 2>/dev/null | wc -l)"
echo "  OTHER (CEIL_2D): $(ls -1 ../uncompatibleData/OTHER/*.tsp 2>/dev/null | wc -l)"
