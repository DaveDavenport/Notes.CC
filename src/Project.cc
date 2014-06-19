#include <iostream>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>

#include <Project.h>

void Project::set_parent(Project *parent)
{
	assert(this->parent == nullptr);
	assert(parent != nullptr);
	this->parent = parent;
}


bool Project::is_root()
{
	return (this->parent == nullptr);
}

Project::Project(const char *name)
{
	assert(name != nullptr);
	this->name = name;
}
Project::~Project()
{
	for ( auto child : child_projects ) {
		delete child;
	}
}

std::string Project::get_name()
{
	if(parent != nullptr  && !parent->is_root()) {
		return parent->get_name()+"."+name;
	}
	return name;
}

void Project::add_subproject(Project *child)
{
	this->child_projects.push_back(child);
	child->set_parent(this);
}

void Project::add_note(Note *note)
{
	this->notes.push_back(note);
}

void Project::print()
{
	std::cout << "Project: " << this->get_name() << std::endl;
	std::cout << "         " << this->notes.size()<< " # notes." << std::endl;
	for ( auto pr : child_projects )
	{
		pr->print();
	}
}

std::string Project::get_path()
{
	return parent->get_path()+"/"+name;
}

void Project::list_projects()
{
    if(!this->is_root()) {
        std::cout << this->get_name() << std::endl;
    }
    for ( auto pr : child_projects )
	{
		pr->list_projects();
	}
}
