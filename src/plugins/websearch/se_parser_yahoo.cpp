/**
 * The Seeks proxy and plugin framework are part of the SEEKS project.
 * Copyright (C) 2010 Emmanuel Benazera, juban@free.fr
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
  
#include "se_parser_yahoo.h"
#include "miscutil.h"

#include <strings.h>
#include <iostream>

using sp::miscutil;

namespace seeks_plugins
{
   se_parser_yahoo::se_parser_yahoo()
     :_start_results(false),_begin_results(false),_title_flag(false),
      _summary_flag(false)
     {
	
     }
   
   se_parser_yahoo::~se_parser_yahoo()
     {
	
     }
   
   void se_parser_yahoo::start_element(parser_context *pc,
				       const xmlChar *name,
				       const xmlChar **attributes)
     {
	const char *tag = (const char*)name;
	
	if (strcasecmp(tag,"div") == 0)
	  {
	     const char *a_id = se_parser::get_attribute((const char**)attributes,"id");
	     const char *a_class = se_parser::get_attribute((const char**)attributes,"class");
	     
	     if (!_start_results && a_id && strcasecmp(a_id,"web") == 0)
	       _start_results = true;
	     else if (_begin_results && a_class && strcasecmp(a_class,"abstr") == 0)
	       _summary_flag = true;
	     else if (_begin_results && a_class && strncasecmp(a_class,"res",3) == 0)		      
	       {
		  // create new snippet.
		  search_snippet *sp = new search_snippet(_count++);
		  sp->_engine |= std::bitset<NSEs>(SE_YAHOO);
		  pc->_current_snippet = sp;
		  pc->_snippets->push_back(pc->_current_snippet);
	       }
	  }
	/* else if (_begin_results && strcasecmp(tag,"li") == 0)
	  {
	  } */
	else if (_start_results && strcasecmp(tag,"ol") == 0)
	  {
	     _begin_results = true;
	  }
	else if (_begin_results && strcasecmp(tag,"h3") == 0)
	  {
	     _title_flag = true;
	  }
	else if (strcasecmp(tag,"a") == 0)
	  {
	     const char *a_link = se_parser::get_attribute((const char**)attributes,"href");
	     
	     if (a_link)
	       {
		  if (_title_flag && pc->_current_snippet)
		    {
		       pc->_current_snippet->set_url(a_link);
		       pc->_current_snippet->_cite = std::string(a_link);
		    }
		  else if (_begin_results && pc->_current_snippet)
		    pc->_current_snippet->_cached = std::string(a_link);
	       }
	  }	  
     }
   
   void se_parser_yahoo::characters(parser_context *pc,
				    const xmlChar *chars,
				    int length)
     {
	handle_characters(pc, chars, length);
     }
   
   void se_parser_yahoo::cdata(parser_context *pc,
			       const xmlChar *chars,
			       int length)
     {
	//handle_characters(pc, chars, length);
     }
   
   void se_parser_yahoo::handle_characters(parser_context *pc,
					   const xmlChar *chars,
					   int length)
     {
	if (_title_flag)
	  {
	     std::string a_chars = std::string((char*)chars);
	     miscutil::replace_in_string(a_chars,"\n"," ");
	     miscutil::replace_in_string(a_chars,"\r"," ");
	     _title += a_chars;
	  }
	else if (_summary_flag)
	  {
	     std::string a_chars = std::string((char*)chars);
	     miscutil::replace_in_string(a_chars,"\n"," ");
	     miscutil::replace_in_string(a_chars,"\r"," ");
	     miscutil::replace_in_string(a_chars,"-"," ");
	     _summary += a_chars;
	  }
	
     }
   
   void se_parser_yahoo::end_element(parser_context *pc,
				     const xmlChar *name)
     {
	const char *tag = (const char*)name;
	
	if (_begin_results && strcasecmp(tag,"ol") == 0)
	  {
	     _begin_results = false;
	  }
	else if (_title_flag && strcasecmp(tag,"h3") == 0)
	  {
	     _title_flag = false;
	     pc->_current_snippet->_title = _title;
	     _title = "";
	  }
	else if (_summary_flag && strcasecmp(tag,"div") == 0)
	  {
	     _summary_flag = false;
	     pc->_current_snippet->set_summary(_summary);
	     _summary = "";
	  }
		
     }
   
} /* end of namespace. */
