data_simple  = /$[a-zA-Z0-9:/#-]+(\.[a-zA-Z0-9-]+)*/
data_complex = /${[a-zA-Z0-9:/# -]+(\.[a-zA-Z0-9- ]+)*}/

Data: data_simple  {{ return ({ tree[0][1..] }); }}
Data: data_complex {{ return ({ tree[0][2..strlen(tree[0]) - 2] }); }}
