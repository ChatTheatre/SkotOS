# define XML			"/usr/XML/sys/xml"

# define XML_ELEMENT		"xml_element"
# define COL_ELEMENT		1
# define XML_SAMREF		"xml_samref"
# define COL_SAMREF		2
# define XML_PCDATA		"xml_pcdata"
# define COL_PCDATA		17
# define XML_MIXED		"xml_mixed"
# define XML_BOOL		"xml_bool"

# define NewXMLElement(d)       new_object("/usr/XML/data/element", d)
# define NewXMLPCData(d)        new_object("/usr/XML/data/pcdata", d)
# define NewXMLSAMRef(d)        new_object("/usr/XML/data/samref", d)
