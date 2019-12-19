//File: Directory.cpp
//Brief: Directory wraps over any class that implements a contract like 
//       art::TFileDirectory to expose the same contract without creating 
//       any sub-TDirectoires.  Instead, Directory::make<>() appends its 
//       name to objects' names to give some strong assurances of name 
//       uniqueness.  Directory might be useful in adapting code that 
//       normally relies on TFileDirectory-like objects to work with 
//       post-processing software like Monet that can't deal with nested 
//       TDirectories. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

//util includes
#include "util/Directory.h"

//ROOT includes
#include "TFile.h"

namespace util  
{
  //Define member functions out of class body for cleanliness
  Directory::Directory(TFile* dir): fBaseDir(dir), fName("")
  {
  }
                                                                                                                              
  Directory Directory::mkdir(const std::string& name)
  {
    Directory child(name, *this);
    return child;
  }
                                                                                                                              
  Directory::Directory(const std::string& name, Directory& parent): fBaseDir(parent.fBaseDir), 
                                                                    fName(parent.fName+name+Separator)
  {
  }
}
