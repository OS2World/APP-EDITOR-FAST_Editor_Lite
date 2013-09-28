/*
** Module   :BINDERY.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  01/04/2004	Created
**
*/

#include <collect.h>

#ifndef __BINDERY_H
#define __BINDERY_H

//-----------------------------------------
// Base Variable class
//-----------------------------------------

class Variable;

typedef Variable* PVariable;

class Variable
{
		char* pName;
		char* pValue;

	protected:

		void fromInt(int i);
		int asInt();

	public:
		Variable(char* aName);
		virtual ~Variable();

		char* getName()			{ return pName;}
		virtual char* getValue(){ return pValue;}
		virtual int setValue(char*);

		virtual char* getValue(char*) 		{ return 0;}	//group access
		virtual int setValue(char*, char*)	{ return 1;}

		virtual int isPermanent()	{ return 0; }
		virtual int isGroup()		{ return 0; }
};

//-----------------------------------------
// Registry (all editor variables/settings)
//-----------------------------------------

class Bindery: public SortedCollection
{
        virtual void Free(Ptr p)    { delete ((Variable *)p);}
        virtual int Compare(Ptr p1, Ptr p2);

        Variable* LocateVar(char*, int);

	public:

		Bindery();
		virtual ~Bindery();

		void delVar(char*);
		void dropVar(char*);
		void setVar(char*, char*);
		char* getVar(char*);

		void LoadDefaults();

		void setDefaultProfile();
		void setCurrentProfile();
};

#endif  /*__BINDERY_H*/

