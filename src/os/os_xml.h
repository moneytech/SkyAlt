/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-03-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

void OsXml_initGlobal(void)
{
	xmlInitParser();
}

void OsXml_freeGlobal(void)
{
	xmlCleanupParser();
}

typedef struct OsXml_s
{
	xmlDocPtr document;
}OsXml;

typedef struct OsXmlMark_s
{
	xmlXPathContextPtr context;
	xmlXPathObjectPtr seg;
}OsXmlMark;

static OsXml* _OsXml_new(xmlDocPtr document)
{
	OsXml* self = Os_malloc(sizeof(OsXml));
	self->document = document;
	return self;
}

OsXml* OsXml_newFile(const char* path)
{
	return _OsXml_new(xmlParseFile(path));
}

OsXml* OsXml_newMemory(const char* buffer, int size)
{
	return _OsXml_new(xmlParseMemory(buffer, size));
}

OsXml* OsXml_newString(const UCHAR* str)
{
	return _OsXml_new(xmlParseDoc(str));
}

void OsXml_delete(OsXml* self)
{
	if (self->document)xmlFreeDoc(self->document);
	Os_free(self, sizeof(OsXml));
}

OsXmlMark* OsXml_createMark(OsXml* self, const char* str, const char* prefix, const char* url)
{
	OsXmlMark* seg = Os_malloc(sizeof(OsXmlMark));

	seg->context = xmlXPathNewContext(self->document);
	xmlXPathRegisterNs(seg->context, (xmlChar*)prefix, (xmlChar*)url);
	seg->seg = xmlXPathEvalExpression((xmlChar*)str, seg->context);

	return seg;
}

void OsXmlMark_delete(OsXmlMark* self)
{
	if (self)
	{
		if (self->seg)xmlXPathFreeObject(self->seg);
		if (self->context)xmlXPathFreeContext(self->context);
		Os_free(self, sizeof(OsXmlMark));
	}
}

UBIG OsXmlMark_num(OsXmlMark* self)
{
	return self->seg->nodesetval->nodeNr;
}

BOOL OsXmlMark_isElement(OsXmlMark* self, UBIG i)
{
	return (self->seg->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE);
}

BOOL OsXmlMark_isEmpty(OsXmlMark* self)
{
	return !self || xmlXPathNodeSetIsEmpty(self->seg->nodesetval);
}

BOOL OsXmlMark_hasProp(OsXmlMark* self, UBIG i, const char* str)
{
	return xmlHasProp(self->seg->nodesetval->nodeTab[i], (xmlChar*)str) != 0;
}

const char* OsXmlMark_getProp(OsXmlMark* self, UBIG i, const char* str)
{
	return (const char*)xmlGetProp(self->seg->nodesetval->nodeTab[i], (xmlChar*)str);
}

const char* OsXmlMark_getValue(const OsXmlMark* self)
{
	return (const char*)self->seg->nodesetval->nodeTab[0]->content;
}

OsXmlMark* OsXmlMark_createMark(OsXmlMark* self, OsXml* xml, const char* prefix, const char* url)
{
	OsXmlMark* seg = Os_malloc(sizeof(OsXmlMark));

	seg->context = xmlXPathNewContext(xml->document);
	xmlXPathRegisterNs(seg->context, (xmlChar*)prefix, (xmlChar*)url);

	//OsXmlMark_reset(seg, self, i, str);
	//seg->context->node = self->seg->nodesetval->nodeTab[i];
	//seg->seg = xmlXPathEvalExpression((xmlChar*)str, seg->context);

	return seg;
}
void OsXmlMark_setContext(OsXmlMark* self, OsXmlMark* orig, UBIG i, const char* str)
{
	self->context->node = orig->seg->nodesetval->nodeTab[i];
	self->seg = xmlXPathEvalExpression((xmlChar*)str, self->context);
}



void OsXml_testGpx(const char* path)
{
	const char* prefix = "gpx";
	const char* url = "http://www.topografix.com/GPX/1/1";

	OsXml* gpx = OsXml_newFile(path);

	OsXmlMark* tracks = OsXml_createMark(gpx, "//gpx:trk/gpx:trkseg", prefix, url);
	if (tracks)
	{
		OsXmlMark* points = OsXmlMark_createMark(tracks, gpx, prefix, url);
		OsXmlMark* elevations = OsXmlMark_createMark(points, gpx, prefix, url);
		OsXmlMark* time = OsXmlMark_createMark(points, gpx, prefix, url);

		BIG i;
		for (i = 0; i < OsXmlMark_num(tracks); i++)
		{
			if (OsXmlMark_isElement(tracks, i))
			{
				OsXmlMark_setContext(points, tracks, i, "gpx:trkpt");
				if (!OsXmlMark_isEmpty(points))
				{
					BIG j;
					for (j = 0; j < OsXmlMark_num(points); j++)
					{
						if (OsXmlMark_hasProp(points, j, "lat") && OsXmlMark_hasProp(points, j, "lon"))
						{
							double lt = Os_atof(OsXmlMark_getProp(points, j, "lat"));
							double ln = Os_atof(OsXmlMark_getProp(points, j, "lon"));
							printf("%f-%f-", lt, ln);
						}

						OsXmlMark_setContext(elevations, points, j, "gpx:ele/text()");
						if (!OsXmlMark_isEmpty(elevations))
						{
							double ele = Os_atof(OsXmlMark_getValue(elevations));
							printf("%f", ele);
							//...
						}

						OsXmlMark_setContext(time, points, j, "gpx:time/text()");
						if (!OsXmlMark_isEmpty(time))
						{
							const char* tm = OsXmlMark_getValue(time);
							UNI* str = Std_newUNI_char(tm);
							OsDate date = OsDate_initFromString(str, OsDate_ISO);
							Std_deleteUNI(str);

							char out[64];
							OsDate_getStringDateISO(&date, out);
							printf(" %s ", out);
							OsDate_getStringTime3(&date, out);
							printf(" %s\n", out);
							

						}
					}
				}
			}
		}

		OsXmlMark_delete(elevations);
		OsXmlMark_delete(time);
		OsXmlMark_delete(points);
	}

	OsXmlMark_delete(tracks);
	OsXml_delete(gpx);
}
