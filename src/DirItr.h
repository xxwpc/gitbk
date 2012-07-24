#include "NodeAttr.h"



class DirectoryIterator
{
public:
   DirectoryIterator( const std::string & );

   bool next( );

   const NodeAttr & attr( ) const
   {
      return entrys[idx];
   }

protected:
   std::vector<NodeAttr>         entrys;
   size_t                        idx;
};



class RDirectoryIterator
{
public:
   RDirectoryIterator( const std::string & );

   bool next( );

   const NodeAttr & attr( ) const
   {
      return top->attr( );
   }

   const boost::filesystem::path &dir( ) const
   {
      return _path;
   }

protected:
   void push( const char * );

protected:
   std::list<DirectoryIterator> stack;

   DirectoryIterator *top;

   mode_t      _lastMode = 0;

   boost::filesystem::path       _path;
};

