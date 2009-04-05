/*
 * Copyright (C) 2006,2009 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <zim/file.h>
#include <zim/article.h>
#include <cxxtools/log.h>
#include <zim/fileiterator.h>

log_define("zim.file")

namespace zim
{
  Dirent File::getDirent(size_type idx)
  {
    log_trace("File::getDirent(" << idx << ')');

    return impl->getDirent(idx);
  }

  Article File::getArticle(size_type idx) const
  {
    return Article(*this, idx);
  }

  Article File::getArticle(char ns, const QUnicodeString& title, bool collate)
  {
    const_iterator it = find(ns, title, collate);
    return it == end() ? Article() : *it;
  }

  bool File::hasNamespace(char ch)
  {
    size_type off = getNamespaceBeginOffset(ch);
    return off < getCountArticles() && getDirent(off).getNamespace() == ch;
  }

  File::const_iterator File::begin()
  { return const_iterator(this, 0); }

  File::const_iterator File::end()
  { return const_iterator(this, getCountArticles()); }

  File::const_iterator File::find(char ns, const QUnicodeString& title, bool collate)
  {
    log_debug("find article " << ns << " \"" << title << "\", " << collate << " in file \"" << getFilename() << '"');

    if (getNamespaces().find(ns) == std::string::npos)
    {
      log_debug("namespace " << ns << " not found");
      return end();
    }

    size_type l = getNamespaceBeginOffset(ns);
    size_type u = getNamespaceEndOffset(ns);

    unsigned itcount = 0;
    while (u - l > 1)
    {
      ++itcount;
      size_type p = l + (u - l) / 2;
      Dirent d = getDirent(p);

      int c = ns < d.getNamespace() ? -1
            : ns > d.getNamespace() ? 1
            : (collate ? title.compareCollate(QUnicodeString(d.getTitle()))
                       : title.compare(QUnicodeString(d.getTitle())));
      if (c < 0)
        u = p;
      else if (c > 0)
        l = p;
      else
      {
        log_debug("article found after " << itcount << " iterations in file \"" << getFilename() << "\" at index " << p);
        return const_iterator(this, p);
      }
    }

    Dirent d = getDirent(l);
    int c = collate ? title.compareCollate(QUnicodeString(d.getTitle()))
                    : title.compare(QUnicodeString(d.getTitle()));
    if (c == 0)
    {
      log_debug("article found after " << itcount << " iterations in file \"" << getFilename() << "\" at index " << l);
      return const_iterator(this, l);
    }

    log_debug("article not found (\"" << d.getTitle() << "\" does not match)");
    return const_iterator(this, u);
  }

}