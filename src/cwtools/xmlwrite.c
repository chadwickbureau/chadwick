/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/xmlwrite.c
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "xmlwrite.h"


XMLDoc *
xml_document_create(FILE *f, char *root)
{
  XMLDoc *doc = (XMLDoc *) malloc(sizeof(XMLDoc));
  doc->f = f;
  doc->root = (XMLNode *) malloc(sizeof(XMLNode));
  doc->root->depth = 0;
  doc->root->open = 1;
  doc->root->has_children = 0;
  doc->root->name = root;
  doc->root->indent = (char *) malloc(1);
  doc->root->indent[0] = '\0';
  doc->root->f = f;
  doc->root->next = NULL;

  fprintf(f, "<?xml version=\"1.0\"?>\n");
  fprintf(f, "<%s", root);

  return doc;
}

void 
xml_document_cleanup(XMLDoc *doc)
{
  XMLNode *node = doc->root;

  if (node->open) {
    xml_node_close(node);
  }

  while (node != NULL) {
    XMLNode *next = node->next;
    free(node->indent);
    free(node);
    node = next;
  }
  free(doc);
}

XMLNode *
xml_node_open(XMLNode *parent, char *name)
{
  int i;

  if (parent->next) {
    /* A child node entry has been allocated.  Is it still open?
     * If it is, recursively close all open children.
     */
    if (parent->next->open) {
      xml_node_close(parent->next);
    }
  }
  else {
    parent->next = (XMLNode *) malloc(sizeof(XMLNode));
    parent->next->depth = parent->depth + 1;
    parent->next->has_children = 0;
    parent->next->f = parent->f;
    parent->next->next = NULL;
    parent->next->indent = (char *) malloc(sizeof(char)*2*parent->next->depth + 1);
    for (i = 0; i < 2 * parent->next->depth; i++) {
      parent->next->indent[i] = ' ';
    }
    parent->next->indent[2*parent->next->depth] = '\0';
  }

  if (!parent->has_children) {
    /* We still need to close the opening tag */
    fprintf(parent->f, ">\n");
    parent->has_children = 1;
  }

  parent->next->name = name;
  parent->next->open = 1;
  fprintf(parent->next->f, "%s<%s", parent->next->indent, name);
  return parent->next;
}

void xml_node_close(XMLNode *node)
{
  if (!node->open) {
    return;
  }

  if (node->next && node->next->open) {
    xml_node_close(node->next);
  }

  if (node->has_children) {
    fprintf(node->f, "%s</%s>\n", node->indent, node->name);
  }
  else {
    fprintf(node->f, "/>\n");
  }

  node->has_children = 0;
  node->open = 0;
}

void 
xml_node_cdata(XMLNode *node, char *data)
{
  if (node->next && node->next->open) {
    xml_node_close(node->next);
  }

  if (!node->has_children) {
    /* We still need to close the opening tag */
    fprintf(node->f, ">\n");
    node->has_children = 1;
  }
  fprintf(node->f, "%s  %s\n", node->indent, data);
}

void
xml_node_attribute(XMLNode *node, char *attr, char *value)
{
  if (!node->open) return;
  fprintf(node->f, " %s=\"%s\"", attr, value);
}

void
xml_node_attribute_int(XMLNode *node, char *attr, int value)
{
  if (!node->open) return;
  fprintf(node->f, " %s=\"%d\"", attr, value);
}

void
xml_node_attribute_posint(XMLNode *node, char *attr, int value)
{
  if (!node->open || value < 0) return;
  fprintf(node->f, " %s=\"%d\"", attr, value);
}

void
xml_node_attribute_fmt(XMLNode *node, char *attr, char *format, ...) 
{
  va_list argp;
  char value[1024];

  va_start(argp, format);
  vsprintf(value, format, argp);
  va_end(argp);
  fprintf(node->f, " %s=\"%s\"", attr, value);
}
