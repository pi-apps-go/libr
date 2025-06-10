/*
 *
 *  Copyright (c) 2010 Erich Hoover
 *
 *  libr "one canvas" - Handle multiple icons stored in a single "one canvas" 
 *                      SVG document.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * To provide feedback, report bugs, or otherwise contact me:
 * ehoover at mines dot edu
 *
 */
 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define FALSE 0
#define TRUE  1

typedef struct {
	double x;
	double y;
	double width;
	double height;
	int icon_width;
	int icon_height;
} IconSVG;

typedef enum {
	STATUS_FINDSVG,
	STATUS_FINDMETADATA,
	STATUS_FINDPUBLISHER_START,
	STATUS_FINDPUBLISHER_STOP,
	STATUS_FINDHIDDEN,
	STATUS_FINDBOUNDS,
	STATUS_FAILED,
	STATUS_DONE,
} eStatus;

typedef struct {
	IconSVG **iconlist;
	int iconlist_num;
	eStatus status;
	
	char *hidden_stop;
	char *hidden_start;
	char *publisher_stop;
	char *publisher_start;
	char *coordinate_stop;
	char *coordinate_start;
} OneCanvasIconInfo;

/*
 * Find the start of the next XML tag (search for '<')
 */
static inline char *xml_nextTag(char *c)
{
	c++;
	if(c == NULL)
		return NULL;
	return strchr(c, '<');
}

/*
 * Pull out the name/type of a tag.
 */
static inline char *xml_getTagName(char *c)
{
	char *tag_end = NULL, *tag_space, *tag_close, *tag_feed, *tag_line;
	static char tagname[20];
	int tag_len;
	
	if(++c == NULL)
		return NULL;
	tag_space = strchr(c, ' ');
	tag_close = strchr(c, '>');
	tag_feed = strchr(c, '\r');
	tag_line = strchr(c, '\n');
	if(tag_space)
		tag_end = tag_space;
	if(tag_close && tag_end > tag_close)
		tag_end = tag_close;
	if(tag_feed && tag_end > tag_feed)
		tag_end = tag_feed;
	if(tag_line && tag_end > tag_line)
		tag_end = tag_line;
	if(!tag_end)
		return NULL;
	tag_len = tag_end - c;
	tag_len = tag_len > 19 ? 19 : tag_len;
	strncpy(tagname, c, tag_len);
	tagname[tag_len] = '\0';
	return tagname;
}

/*
 * Find the position in the string corresponding to a particular named attribute.
 */
static inline char *xml_getTagAttributePtr(char *c, char *attrname)
{
	char *end, *name;
	int found;
	
	if(++c == NULL)
		return NULL;
	end = strchr(c, '>');
	while(c < end)
	{
		int name_len;
		char *equal;

		equal = c = strchr(c, '=');
		if(c == NULL)
			break;
		c++;
		name = equal;
		while(name[0] != ' ' && name[0] != '\t' && name[0] != '\n')
			name--;
		name++; /* don't include the space */
		name_len = equal-name;
		if(name_len != strlen(attrname))
			continue;
		if(strncasecmp(attrname, name, name_len) == 0)
		{
			found = TRUE;
			break;
		}
	}
	if(!found)
		return NULL;
	return c-strlen(attrname)-1;
}

/*
 * Return the value of an XML tag's named attribute.
 */
static inline char *xml_getTagAttribute(char *c, char *attrname)
{
	char *data_end;
	int data_len;
	char *attr;

	c = xml_getTagAttributePtr(c, attrname);
	if(c == NULL)
		return NULL;
	c+=strlen(attrname); /* skip the name */
	c+=2; /* skip the equals sign and the quote */
	data_end = strchr(c, '"');
	data_len = data_end - c;
	attr = (char *) malloc(data_len+1);
	strncpy(attr, c, data_len);
	attr[data_len] = '\0';
	return attr;
}

/*
 * Find the value of an XML tag attribute and convert it to a number.
 */ 
static inline double xml_getTagAttributeFloat(char *c, char *attrname)
{
	char *value = xml_getTagAttribute(c, attrname);
	double ret;

	if(!value)
		return nan("nan");
	sscanf(value, "%lf", &ret);
	free(value);
	return ret;
}

/*
 * Match the beginning an XML tag by "id" (preferred) or Inkscape's
 * label (undesireable but acceptable).
 */
static inline char *xml_idMatchStart(char *stream_pos, char *layer_name)
{
	char *id_acceptable = xml_getTagAttribute(stream_pos, "inkscape:label");
	char *id_preferred = xml_getTagAttribute(stream_pos, "id");

	if(id_preferred && strncasecmp(id_preferred, layer_name, strlen(layer_name)) == 0)
	{
		free(id_acceptable);
		return id_preferred;
	}
	if(id_acceptable && strncasecmp(id_acceptable, layer_name, strlen(layer_name)) == 0)
	{
		free(id_preferred);
		return id_acceptable;
	}
	free(id_acceptable);
	free(id_preferred);
	return NULL;
}

/*
 * Match the entirety of an XML tag by "id" (preferred) or Inkscape's
 * label (undesireable but acceptable).
 */
static inline int xml_idMatch(char *stream_pos, char *layer_name)
{
	char *id_acceptable = xml_getTagAttribute(stream_pos, "inkscape:label");
	char *id_preferred = xml_getTagAttribute(stream_pos, "id");
	int ret = FALSE;

	if((id_preferred && strcasecmp(id_preferred, layer_name) == 0)
	    || (id_acceptable && strcasecmp(id_acceptable, layer_name) == 0))
		ret = TRUE;
	free(id_acceptable);
	free(id_preferred);
	return ret;
}

/*
 * Strip all the XML tags from a string and return only the data not
 * contained within any tags.
 */
static inline char *xml_stripTags(char *data, int len)
{
	char *ret = (char *) malloc(len+1);
	char *tag_left, *tag_right;

	memcpy(ret, data, len+1);
	ret[len] = '\0';
	while((tag_left = strchr(ret, '<')) != NULL)
	{
		tag_right = strchr(ret, '>');
		memmove(tag_left, tag_right+1, strlen(ret)-(tag_right-ret));
	}
	return ret;
}

/*
 * Return the information for all of the icons within a "one-canvas"
 * data stream.
 */
OneCanvasIconInfo onecanvas_geticons(char *stream)
{
	eStatus status = STATUS_FINDSVG;
	unsigned int stream_size = 0;
	OneCanvasIconInfo info;
	char *publisher = NULL;
	char *stream_pos;
	int i;

	memset(&info, 0, sizeof(info)); 
	stream_pos = stream;
	while(stream_pos)
	{
		char *name = xml_getTagName(stream_pos);

		if(!name)
		{
			stream_pos = xml_nextTag(stream_pos);
			continue;
		}
		switch(status)
		{
			case STATUS_FINDSVG:
			{
				if(strcasecmp(name, "svg") == 0)
				{
					info.coordinate_start = xml_getTagAttributePtr(stream_pos, "x");
					info.coordinate_stop = xml_getTagAttributePtr(stream_pos, "viewBox");
					if(info.coordinate_start == NULL || info.coordinate_stop == NULL)
					{
						status = STATUS_FAILED;
						break;
					}
					info.coordinate_stop = strchr(info.coordinate_stop, '"')+1;
					info.coordinate_stop = strchr(info.coordinate_stop, '"')+1;
					status = STATUS_FINDMETADATA;
				}
			} break;
			case STATUS_FINDMETADATA:
			{
				if(strcasecmp(name, "metadata") == 0)
				{
					status = STATUS_FINDPUBLISHER_START;
				}
				else if(strcasecmp(name, "/svg") == 0)
				{
					status = STATUS_FAILED;
				}
			} break;
			case STATUS_FINDPUBLISHER_START:
			{
				if(strcasecmp(name, "dc:publisher") == 0)
				{
					status = STATUS_FINDPUBLISHER_STOP;
					info.publisher_start = stream_pos + strlen("<dc:publisher>");
				}
				else if(strcasecmp(name, "/metadata") == 0)
				{
					status = STATUS_FAILED;
				}
			} break;
			case STATUS_FINDPUBLISHER_STOP:
			{
				if(strcasecmp(name, "/dc:publisher") == 0)
				{
					info.publisher_stop = stream_pos;
					publisher = xml_stripTags(info.publisher_start, info.publisher_stop-info.publisher_start);
					if(strcasecmp(publisher, "one-canvas") == 0)
						status = STATUS_FINDHIDDEN;
					else
						status = STATUS_FAILED;
				}
				else if(strcasecmp(name, "/metadata") == 0)
				{
					status = STATUS_FAILED;
				}
			} break;
			case STATUS_FINDHIDDEN:
			{
				if(strcasecmp(name, "g") == 0)
				{
					if(xml_idMatch(stream_pos, "hidden"))
					{
						char *style_start;

						info.hidden_start = stream_pos;
						info.hidden_stop = info.hidden_start;
						style_start = xml_getTagAttributePtr(stream_pos, "style");
						if(style_start)
						{
							info.hidden_start = style_start;
							info.hidden_stop = strchr(style_start, '"')+1;
							info.hidden_stop = strchr(info.hidden_stop, '"')+1;
						}
						else
						{
							info.hidden_start += strlen("<g ");
							info.hidden_stop += strlen("<g ");
						}
						status = STATUS_FINDBOUNDS;
					}
				}
			} break;
			case STATUS_FINDBOUNDS:
			{
				if(strcasecmp(name, "rect") == 0)
				{
					char *layer_name = xml_idMatchStart(stream_pos, "iconlayer-");

					if(layer_name != NULL)
					{
						IconSVG *icon = (IconSVG *) malloc(sizeof(IconSVG));

						icon->x = xml_getTagAttributeFloat(stream_pos, "x");
						icon->y = xml_getTagAttributeFloat(stream_pos, "y");
						icon->width = xml_getTagAttributeFloat(stream_pos, "width");
						icon->height = xml_getTagAttributeFloat(stream_pos, "height");
						sscanf(layer_name, "iconlayer-%dx%d", &(icon->icon_width), &(icon->icon_height));
						free(layer_name);
						status = STATUS_FINDBOUNDS;
						info.iconlist = (IconSVG **) realloc(info.iconlist, (info.iconlist_num+1)*sizeof(IconSVG *));
						info.iconlist[info.iconlist_num] = icon;
						info.iconlist_num++;
					}
				}
				else if(strcasecmp(name, "/g") == 0)
				{
					status = STATUS_DONE;
				}
			} break;
			default:
				break;
		}
		if(status == STATUS_DONE || status == STATUS_FAILED)
			break;
		stream_pos = xml_nextTag(stream_pos);
	}
	free(publisher);
	info.status = status;
	return info;
}

/*
 * Obtain a single icon from the "one-canvas" stream corresponding
 * to a particular icon size.
 */
char *onecanvas_geticon_bysize(char *icon_data, int requested_size)
{
	OneCanvasIconInfo info = onecanvas_geticons(icon_data);
	char *ret = NULL;
	int i;
	
	if(info.status == STATUS_DONE && info.iconlist_num > 0)
	{
		int closest_diff = abs(info.iconlist[0]->icon_width - requested_size);
		int tocoord_length, topubl_length, tohidden_length;
		int icon_id = 0;
		IconSVG *icon;
		int ret_max;
		
		for(i=0;i<info.iconlist_num;i++)
		{
			int size_diff = abs(info.iconlist[i]->icon_width - requested_size);
			
			if(size_diff < closest_diff)
			{
				closest_diff = size_diff;
				icon_id = i;
			}
		}
		icon = info.iconlist[icon_id];
 		/* Note: 200 characters is a very generous over estimate for the data we add in */
		ret_max = strlen(icon_data)+1+200;
		ret = (char *) malloc(ret_max);
		tocoord_length = info.coordinate_start-icon_data;
		snprintf(ret, ret_max, "%.*s", tocoord_length, icon_data);
		/* Output the coordinates of the icon */
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "\nx=\"0px\"\ny=\"0px\"\n");
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "width=\"%d\"\n", icon->icon_width);
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "height=\"%d\"\n", icon->icon_height);
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "viewBox=\"%lf %lf %lf %lf\"\n", icon->x, icon->y, icon->width, icon->height);
		topubl_length = info.publisher_start-info.coordinate_stop;
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "%.*s", topubl_length, info.coordinate_stop);
		/* Hide the "hidden" layer */
		tohidden_length = info.hidden_start-info.publisher_stop;
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "%.*s", tohidden_length, info.publisher_stop);
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "\ndisplay=\"none\"\n");
		/* Output the rest of the document */
		snprintf(&ret[strlen(ret)], ret_max-strlen(ret), "%s", info.hidden_stop);
	}
	for(i=0;i<info.iconlist_num;i++)
		free(info.iconlist[i]);
	free(info.iconlist);
	return ret;
}