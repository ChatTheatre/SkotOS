private string denew_keyword(string keyword)
{
	if (strlen(keyword) >= 3) {
		if (keyword[0 .. 2] == "new") {
			keyword += "_";
		}
	}

	return keyword;
}

private string denew(string source)
{
	/* remove pesky "new" keyword from lpc source */
	string output;
	string keyword;

	int context;
	int in_escape;
	int in_keyword;
	int pos, sz;

	output = "";
	keyword = "";

	/* contexts */
	/* 0 = plain */
	/* 1 = C comment */
	/* 2 = C++ comment */
	/* 3 = preprocessor directive */
	/* 4 = string */

	sz = strlen(source);

	for (pos = 0; pos < sz; pos++) {
		string ch;

		ch = " ";
		ch[0] = source[pos];

		switch(context) {
		case 0: /* plain */
			switch(source[pos]) {
			case '#':
				output += denew_keyword(keyword);
				keyword = "";
				context = 3;
				break;

			case 'n':
			case 'e':
			case 'w':
			case '_':
				/* potential new keyword, buffer it */
				keyword += ch;
				continue;

			case '"':
				output += denew_keyword(keyword);
				keyword = "";
				context = 4;
				break;

			case '/':
				output += denew_keyword(keyword);
				keyword = "";

				if (pos < sz && source[pos + 1] == '*') {
					pos++;
					output += "/*";
					context = 1;
					continue;
				}

				if (pos < sz && source[pos + 1] == '/') {
					pos++;
					output += "//";
					context = 2;
					continue;
				}
				break;

			case '\\':
				output += denew_keyword(keyword);
				keyword = "";

				if (pos < sz && source[pos + 1] == '\n') {
					pos++;
					output += "\\\n";
					continue;
				}
				break;

			case '\n':
				output += denew_keyword(keyword);
				keyword = "";
				break;

			default:
				output += denew_keyword(keyword);
				keyword = "";
				break;
			}
			break;

		case 1: /* C comment */
			switch(source[pos]) {
			case '*':
				if (pos < sz && source[pos + 1] == '/') {
					pos++;
					output += "*/";
					context = 0;
					continue;
				}
				break;
			}
			break;

		case 2: /* C++ comment */
		case 3: /* preprocessor directive */
			/* both of these end the same way */
			switch(source[pos]) {
			case '\\':
				if (pos < sz && source[pos + 1] == '\n') {
					pos++;
					output += "\\\n";
					continue;
				}

			case '\n':
				context = 0;
				break;
			}
			break;

		case 4: /* string */
			switch(source[pos]) {
			case '\\':
				/* it's going to be an escape no matter what */
				if (pos < sz) {
					pos++;
					output += source[pos - 1 .. pos];
					continue;
				}

			case '"':
				context = 0;
				break;
			}
			break;
		}

		output += ch;
	}

	return output;
}
