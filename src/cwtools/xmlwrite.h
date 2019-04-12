/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwtools/xmlwrite.h
 * Utility API for efficiently generating XML output
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef XMLWRITE_H
#define XMLWRITE_H

#include <stdio.h>

/*
 * This file defines a simple API for efficiently generating XML output,
 * with a very small overhead relative to simple printf()s.
 * Thie idea is to keep a linked list of open nodes.  Creating a child
 * node amounts to allocating a new node on the tail of the linked
 * list.
 *
 * Calling code does not have to worry about explicitly closing open nodes.
 * Creating a new sibling node of an existing node closes the previous
 * sibling, including any child nodes of that sibling.  Any remaining
 * open nodes are closed when the document is closed.
 *
 * Implementation notes:
 * - The linked list representing open nodes never deallocates closed
 *   node entries while building the tree, which avoids malloc()/free()
 *   overhead.
 * - Node names are stored as pointers to the original strings, so no
 *   allocation is done.  This means node names should not be changed in
 *   the calling code during the lifetime of the node.
 * - Pretty-printing is hardcoded to an indent of 2 spaces per level.
 */

typedef struct xml_node_struct {
  int depth, open, has_children;
  char *name, *indent;
  FILE *f;
  struct xml_node_struct *next;
} XMLNode;

typedef struct xml_doc {
  FILE *f;
  struct xml_node_struct *root;
} XMLDoc;

/*
 * Create a new XML document outputter, with root element 'root',
 * outputting to file 'f'.  Note that 'root' is simply kept as a
 * pointer internally, so it should not be allocated or overwritten
 * during the life of the document.
 */
XMLDoc *xml_document_create(FILE *f, char *root);

/*
 * Closes any open nodes in the document, and cleans up memory allocated
 * for the document.  The XMLDoc structure itself is freed, but the
 * associated file is not closed.
 */
void xml_document_cleanup(XMLDoc *doc);

/*
 * Closes 'node', including (recursively) all open children.
 * Implementation note: We don't deallocate the child node entries,
 * so they can be re-used later without the malloc()/free() overhead.
 */
void xml_node_close(XMLNode *node);

/*
 * Opens a node named 'name' as a child of 'parent'.  The name
 * is stored as a pointer, so it should not be deallocated or changed
 * during the open lifetime of the node.
 */
XMLNode *xml_node_open(XMLNode *parent, char *name);


/*
 * Print 'data' as CDATA under 'node'.  Implicitly closes
 * any open child notes of 'node'. The pointer to 'data
 * is not stored, so it may be deallocated or modified after the call
 * completes.
 */
void xml_node_cdata(XMLNode *node, char *data);

/*
 * Adds the attribute 'attr' with value 'value' to the open node 'node'.
 * The pointers to 'attr' and 'value' are not stored, so they may
 * be deallocated or modified after the call completes.
 */
void
xml_node_attribute(XMLNode *node, char *attr, char *value);

/*
 * Adds the attribute 'attr' with value 'value' to the open node 'node'.
 * The pointer to 'attr' is not stored, so it may
 * be deallocated or modified after the call completes.
 */
void
xml_node_attribute_int(XMLNode *node, char *attr, int value);

/*
 * Adds the attribute 'attr' with value 'value' to the open node 'node'.
 * If 'value' is negative, the attribute is not added; therefore,
 * this treats negative values as NULLs or 'missing data.'
 * The pointer to 'attr' is not stored, so it may
 * be deallocated or modified after the call completes.
 */
void
xml_node_attribute_posint(XMLNode *node, char *attr, int value);

/*
 * Adds the attribute 'attr' by formatting the va_args using format string
 * 'format'. The pointer to 'attr' is not stored, so it may
 * be deallocated or modified after the call completes.
 */
void
xml_node_attribute_fmt(XMLNode *node, char *attr, char *format, ...);



#endif  /* XMLWRITE_H */
