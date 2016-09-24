static mixed list_front(mixed *list)
{
	if (list[0]) {
		return list[0][1];
	} else {
		error("List empty");
	}
}

static mixed list_back(mixed *list)
{
	if (list[1]) {
		return list[1][1];
	} else {
		error("List empty");
	}
}

static void list_push_front(mixed *list, mixed data)
{
	mixed *node;

	node = ({ nil, data, nil });

	if (list[0]) {
		node[2] = list[0];	/* new node points forward */
		list[0][0] = node;	/* old node points backward */
		list[0] = node;
	} else {
		list[0] = list[1] = node;
	}
}

static void list_push_back(mixed *list, mixed data)
{
	mixed *node;

	node = ({ nil, data, nil });

	if (list[1]) {
		node[0] = list[1];	/* new node points backward */
		list[1][2] = node;	/* old node points forward */
		list[1] = node;
	} else {
		list[0] = list[1] = node;
	}
}
static void list_pop_front(mixed *list)
{
	if (!list[0]) {
		error("List empty");
	}

	if (list[0] == list[1]) {
		list[0] = list[1] = nil;
	} else {
		list[0] = list[0][2];	/* advance */
		list[0][0] = nil;	/* snip */
	}
}

static void list_pop_back(mixed *list)
{
	if (!list[0]) {
		error("List empty");
	}

	if (list[0] == list[1]) {
		list[0] = list[1] = nil;
	} else {
		list[1] = list[1][0];	/* advance */
		list[1][2] = nil;	/* snip */
	}
}

static int list_empty(mixed *list)
{
	return !list[0];
}
