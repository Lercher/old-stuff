#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>

#define WANDER 0

#define VERSION "0.3"

enum setup {
	MAXTERMINE = 32,
	MAXVORLESUNGEN = 8,
	MAXAUSWAHL = 8,
	MAXSTUDENTEN = 500,
};

typedef long num; // Datentyp der Aergerfunktion


#define foreach(x,Class) for((x)=0; (x)<(Class::n); (x)++)


num aerger, gaerger, saerger, diffaerger;
int errors = 0;

class Number {
	enum Numbertype { UNSET, NORMAL, PINF, NINF} type;
	num value;
public:
	Number();
	int isSet()     { return type != UNSET; }
	int isNormal()	{ return type == NORMAL; }
	void setValue(num n)	{ value = n; type=NORMAL; }
	num theValue()	{ return value; }
};

struct {
	int vorl, vontermin;
	num sa, ga;
	int z, n;
	num temperatur;
	int zyklen;
	num penaltyforminimum, penaltyformaximum;
} global;

class Ident {
protected:
	char *name;
	char ident;
public:
	Ident() { name = ""; }
	void setname(const char *n)	{ name = strdup(n); }
	void setident(char i) { ident = i; }
	const char *getname() { return name; }
	const char getident() { return ident; }
};


class Vorlesung : public Ident {
public:
static int n;
	int penaltys, termine;
	int termin[MAXTERMINE];
	Number penalty[MAXAUSWAHL];
	Number other;
	Vorlesung()		{ penaltys = 0; termine = 0;}
	int einTermin();
static int neu(const char i);
static int zu(const char i);
} *Vorlesungen[MAXVORLESUNGEN];

class Termin : public Ident {
public:
static int n;
	Number opt;
	Number minimum;
	Number maximum;
	Number penalty;
	int aktuell;
	int vorlesungsindex;
	Termin() 		{ penalty.setValue(20); aktuell=0; vorlesungsindex = -1;}
	num mma(int akt);
	num aerger();
	num aerger(int diff);
static int neu(const char);
static int zu(const char, const int);
static int gleich(int i, int j);
} *Termine[MAXTERMINE];

class Student {
public:
	char *name;
	int index[MAXVORLESUNGEN][MAXAUSWAHL+1];
	int intermin[MAXVORLESUNGEN];
	Student();
	num aerger();
	num aerger(int vorlesungsindex);
	num aerger(int vorlesungsindex, int wennintermin);
	void preferences(char vorl, const char *likes);
	void checkdouble();
	void wandere();
	void wandere(int);
	void wandere(int,int);
	void undo();
	int frei(int termin);
static int n;
static int neu(const char *nam);
static int einStudent()	{ return rand() % n; }
} *Studenten[MAXSTUDENTEN];

Number::Number()
{
	type = UNSET;
	value = 0;
}

istream& operator >>(istream& is, Number& num)
{   long n;

	is >> n;
	num.setValue(n);
	return is;
}

ostream& operator <<(ostream& os, Number& num)
{
	if (num.isNormal())
		os << num.theValue();
	else if (num.isSet())
		os << "(isSet)";
	else
		os << "(unset)";
	return os;
}

int Vorlesung::n = 0;

int Vorlesung::neu(const char i)
{   Vorlesung *v;

	if (Vorlesung::n >= MAXVORLESUNGEN)
	{
		cerr << "+++ Zu viele Vorlesungen." << endl;
		exit(2);
	}
	v = Vorlesungen[Vorlesung::n] = new Vorlesung;
	v->setident(i);

	return Vorlesung::n++;
}

int Vorlesung::zu(const char i)
{	int n;

	foreach(n, Vorlesung)
	{
		if (Vorlesungen[n]->ident == i)
			return n;
	}
	return -1;
}

inline int Vorlesung::einTermin()
{
	return termin[rand()%termine];
}


int Termin::n = 0;

int Termin::neu(const char i)
{   Termin *t;

	if (Termin::n >= MAXTERMINE)
	{
		cerr << "+++ Zu viele Termine." << endl;
		exit(2);
	}
	t = Termine[Termin::n] = new Termin;
	t->setident(i);

	return Termin::n++;
}

int Termin::zu(const char i, const int vorl)
{	int n;
//cerr << "===teste Termin::zu===  i=" << i << " vorl=" << vorl << endl;
	foreach(n, Termin)
	{
//cerr << "======================  Termin ident=" << Termine[n]->ident << " Index=" << Termine[n]->vorlesungsindex << endl;
		if (Termine[n]->ident == i && Termine[n]->vorlesungsindex == vorl)
			return n;
	}
	return -1;
}

inline int Termin::gleich(int i, int j)
{
//cerr << "=== gleich(" << i << ", " << j << ") : "
//     << Termine[i]->ident << "==" << Termine[j]->ident << endl;
     
	return Termine[i]->ident == Termine[j]->ident;
}

inline num Termin::mma(int akt)
{	num a=0;

	if (minimum.isNormal() && akt < minimum.theValue())
		a = global.penaltyforminimum * (minimum.theValue()-akt);
	if (maximum.isNormal() && akt > maximum.theValue())
		a += global.penaltyformaximum * (akt-maximum.theValue());
	return a;
}

num Termin::aerger()
{	num a=0;

	if (opt.isNormal())
	{
		a = aktuell - opt.theValue();
		a *= a * penalty.theValue();
	}
	a += mma(aktuell);
	return a;
}

num Termin::aerger(int diff)
{	num a=0;

	if (opt.isNormal())
	{
		a = 2*(aktuell - opt.theValue()) + diff;
		a *= diff * penalty.theValue();
	}
	a += mma(aktuell+diff);
	a -= mma(aktuell);
#if 0
cerr << " dTŽ(" << ident << ")=" << a << " ";
#endif
	return a;
}

Student::Student()
{
	for(int n=0; n<MAXVORLESUNGEN; n++)
		intermin[n] = index[n][0] = -1;
}


int Student::n = 0;

// Die leidigen Umlaute
int Student::neu(const char *st)
{	Student *s;

	if (Student::n >= MAXSTUDENTEN)
	{
		cerr << "+++ Zu viele Studenten." << endl;
		exit(2);
	}
	s = Studenten[Student::n] = new Student;
	if (!s)
	{
		cerr << "+++ Speichermangel." << endl;
		exit(3);
	}
	s->name = strdup(st);
	if (!s->name)
	{
		cerr << "+++ Speichermangel." << endl;
		exit(3);
	}
	return Student::n++;
}



void Student::preferences(char vorl, const char *likes)
{	int v, sll;

	v = Vorlesung::zu(vorl);
	if (v<0)
	{
		cerr << "+++ Unbekannte Vorlesung '" << vorl << "' zum Student" << endl;
		cerr << "    " << name << endl;
		errors++;
		return;
	}
	sll = strlen(likes);
	if (sll > MAXAUSWAHL)
	{
		cerr << "+++ Zu lange Auswahl '" << likes
		     << "' von Terminen zum Student" << endl;
		cerr << "    " << name << endl;
		errors++;
		return;
	}
	for (int n=0; n<sll; n++)
	{
		index[v][n] = Termin::zu(likes[n], v);
		if (index[v][n] < 0)
		{
			cerr << "+++ Unzul„ssiger Termin '" << likes[n]
			     << "' in #" << vorl << likes
			     << " zum Student" << endl;
			cerr << "    " << name << endl;
			errors++;
			return;
		}
	}
	index[v][n] = -1;
}

int Student::frei(int termin)
{	int v;

	foreach(v, Vorlesung)
		if (Termin::gleich(termin, intermin[v]))
			return 0;
	return 1;
}

num Student::aerger(int vorl, int termin)
{	int pi;

	assert(termin >= 0 && termin < Termin::n);
	assert(vorl >= 0 && vorl < Vorlesung::n);
	for (int i=0; (pi=index[vorl][i]) >= 0; i++)
	{
		if (pi == termin)
			return Vorlesungen[vorl]->penalty[i].theValue();
	}
	return Vorlesungen[vorl]->other.theValue();
}

num Student::aerger(int vorl)
{	int termin;

	assert(vorl >= 0 && vorl < Vorlesung::n);
	termin = intermin[vorl];
	if (termin>=0)
		return aerger(vorl, termin);
	return 0;
}

num Student::aerger()
{	int v;
	num a=0;
	
	foreach (v, Vorlesung)
		a += aerger(v);
	return a;
}

void Student::undo()
{
	Termine[intermin[global.vorl]]->aktuell--;
	intermin[global.vorl] = global.vontermin;
	Termine[global.vontermin]->aktuell++;
	::aerger -= diffaerger;
	saerger -= global.sa;
	gaerger -= global.ga;
}

void Student::wandere(int vorl, int nachtermin)
{	int vontermin;

	assert(vorl >= 0 && vorl<Vorlesung::n);
	vontermin = intermin[vorl];
	assert(vontermin >= 0 && vontermin<Termin::n);
	intermin[vorl] = nachtermin;
	assert(nachtermin >= 0 && nachtermin<Termin::n);
	global.sa =
		  aerger(vorl, nachtermin)
		- aerger(vorl, vontermin);
	global.ga =
		  Termine[nachtermin]->aerger(1)
	        + Termine[vontermin]->aerger(-1); // + RICHTIG!
	diffaerger  = global.sa + global.ga;
	::aerger += diffaerger;
	saerger += global.sa;
	gaerger += global.ga;
	global.vorl = vorl; global.vontermin = vontermin;
	Termine[vontermin]->aktuell--;
	Termine[nachtermin]->aktuell++;
#if WANDER
	cerr << "St. "
	     << this << " in "
	     << Vorlesungen[vorl]->getident() << ": "
	     << Termine[vontermin]->getident() <<" -> "
	     << Termine[nachtermin]->getident()
	     << " mit Differenz„rger " << diffaerger;
#endif
}

void Student::wandere(int j)
{	int t, tneu;

	assert(j>=0);
	assert(j<Vorlesung::n);
	for(int sentinel=0; sentinel<100; sentinel++)
	{
	 	tneu = Vorlesungen[j]->einTermin();
		if (frei(tneu))
		{
			wandere(j, tneu);
			return;
		}
	}
}


void Student::wandere()
{	int j, sentinel;

	for(sentinel=0; sentinel<100; sentinel++)
	{
		j = rand() % Vorlesung::n;
		if (intermin[j] >= 0)
		{
			wandere(j);
			break;
		}
	}
}

void Student::checkdouble()
{	int i,j,fault;

	do {
		fault = 0;
		foreach(j, Vorlesung)
		{
			if (intermin[j] >= 0)
			{
				for(i=j+1; i<Vorlesung::n; i++)
				{
//cerr << "=== Fault intermin["<<i<<"]="<<intermin[i]
//     <<" intermin["<<j<<"]="<<intermin[j]<<endl;
					if (Termin::gleich(intermin[i],intermin[j]))
					{
						fault = 1;
					}
				}
			}
		}
		if (fault)
			wandere();
	} while (fault);
}



void parse(char *buf)
{	istrstream line(buf);
	char ident[100], value[100];
static int v=-1, t=-1;

	line >> ident;
	if (0 == strncmp(ident, "!", 1))
	{
		cout << buf+1 << endl;
		return;
	}
	if (0 == strncmp(ident, "#", 1))
		return;
	if (0 == stricmp(ident, "temperatur"))
	{
		line >> global.temperatur;
		return;
	}
	if (0 == stricmp(ident, "maxpenalty"))
	{
		line >> global.penaltyformaximum;
		return;
	}
	if (0 == stricmp(ident, "minpenalty"))
	{
		line >> global.penaltyforminimum;
		return;
	}
	if (0 == stricmp(ident, "koeffizient"))
	{	char ch;
	
		line >> global.z >> ch >>global.n;
		if (ch != '/')
		{
			cerr << "+++ Fehlerhafte Benutzung von Koeffizient. Richtig z.B.: Koeffizient 3/4"
			     << endl;
			errors++;
			return;
		}
		if (global.n==0)
		{
			cerr << "+++ Division durch Null in " << buf << endl;
			errors++;
		}
		if (global.z >= 25 || global.n >= 25 || global.z >= global.n)
		{
			cerr << "+++ Unsinnige Zahlen in " << buf << endl;
			errors++;
		}
		return;
	}
	if (0 == stricmp(ident, "zyklen"))
	{
		line >> global.zyklen;
		return;
	}
	if (0 == stricmp(ident, "vorlesung"))
	{
		line >> value;
		v = Vorlesung::neu(*value);
		t = -1;
		return;
	}
	if (0 == stricmp(ident, "termin"))
	{
		line >> value;
		t = Termin::neu(*value);
		v = -1;
		return;
	}
	if (0 == stricmp(ident, "name"))
	{	char *s;
	
		for(s=buf+line.tellg(); *s && isspace(*s); s++)
			/**/; 
		if (v>=0)
			Vorlesungen[v]->setname(s);
		if (t>=0)
			Termine[t]->setname(s);
		return;
	}
	if (v>=0 && 0 == stricmp(ident, "other"))
	{
		line >> Vorlesungen[v]->other;
		return;
	}
	if (0 == stricmp(ident, "penalty"))
	{
		if (v>=0)
		{
			while(!line.eof())
			{
//cerr << "lese Penalty Nr. " << Vorlesungen[v]->penaltys << endl;
				if (Vorlesungen[v]->penaltys >= MAXAUSWAHL)
				{
					cerr << "+++ Zu viele Penaltys." << endl;
					exit(2);
				}
				line >> Vorlesungen[v]->penalty[Vorlesungen[v]->penaltys++];
			}
			if (Vorlesungen[v]->penaltys)
				Vorlesungen[v]->penaltys--;
			return;
		}
		if (t>0)
		{
			line >> Termine[t]->penalty;
			return;
		}
	}
	if (t>=0 && 0 == stricmp(ident, "opt"))
	{
		line >> Termine[t]->opt;
		return;
	}
	if (t>=0 && 0 == stricmp(ident, "min"))
	{
		line >> Termine[t]->minimum;
		return;
	}
	if (t>=0 && 0 == stricmp(ident, "max"))
	{
		line >> Termine[t]->maximum;
		return;
	}
	if (t>=0 && 0 == stricmp(ident, "zu"))
	{	int idx;
	
		line >> value;
		if ((idx = Vorlesung::zu(*value)) < 0)
		{
			cerr << "+++ Unbekannte Vorlesung " << *value << endl;
			errors++;
		}
		Vorlesungen[idx]->termin[Vorlesungen[idx]->termine++] = t;
		Termine[t]->vorlesungsindex = idx;
		return;
	}
	if (strlen(ident))
	{
		cerr << "+++ Unbekannter identifier: '" << ident << "'";
		if (t>=0)
			cerr << " im Terminmodus.";
		if (v>=0)
			cerr << " im Vorlesungsmodus.";
		cerr << endl;
		errors++;
	}
}

void lies_termine(char *name)
{	ifstream i(name);
	char buf[512];

	if (!i)
	{
		perror(name);
		exit(1);
	}
	while(!i.eof())
	{
		i.get(buf, sizeof(buf));
		i.get();                 // \n entfernen
		parse(buf);
	}
}

void print_termine()
{	int n;
	Vorlesung *v;
	Termin *t;

	foreach(n, Vorlesung)
	{
		v = Vorlesungen[n];
		cout << "Vorlesung " << v->getident() << " mit Namen " << v->getname();
		cout << "  Other " << v->other << " Penaltys ";
		for(int i=0; i<v->penaltys; i++)
			cout << v->penalty[i] << " ";
		cout << endl << "  Termine: ";
		for(i=0; i< v->termine; i++)
			cout << Termine[v->termin[i]]->getident()
			     << "(" << v->termin[i] << ") ";
		cout << endl;
	}
	foreach(n, Termin)
	{
		t = Termine[n];
		assert(t->vorlesungsindex >= 0);
		cout << "Termin " << t->getident()
		     << " zu Vorlesung " << Vorlesungen[t->vorlesungsindex]->getident()
		     << " mit Namen " << t->getname()
		     << endl;
		cout << "  Opt " << t->opt
                     << " Min " << t->minimum
                     << " Max " << t->maximum
                     << " Penalty " << t->penalty
		     << endl;
	}
}

void sparse(char *buf) // Studenten parsen
{	istrstream line(buf);
	char name[512];
	char vorl, ch;
	char likes[50];
	int s;
	
	
	line.get(name, sizeof(name), '#');
	if (strlen(name) == 0 || strlen(name) == strlen(buf))
		return;
	s = Student::neu(name);
	while(1)
	{
		if (line.eof())
			break;
		line >> ch;
		if (!line.bad() && ch == '#')
		{	
			line >> vorl >> likes;
			Studenten[s]->preferences(vorl, likes);
		}
	}
}

void lies_studenten(char *name)
{	ifstream i(name);
	char buf[512];

	if (!i)
	{
		perror(name);
		exit(1);
	}
	while(!i.eof())
	{
		i.get(buf, sizeof(buf));
		i.get();                 // \n entfernen
		sparse(buf);
	}
}

void initialverteilung()
{	Student *s;
	int t, j, i;

	aerger = 0;
	foreach (j, Student)
	{	
		s = Studenten[j];
		foreach (i, Vorlesung)
		{
			if (s->index[i][0] >= 0)
			{
				// in den ersten Vorlesungstermin
				t = s->intermin[i] = Vorlesungen[i]->termin[0];
#if 0
cerr << "Student " << s
     << " fr Vorlesung " << Vorlesungen[i]->getident()
     << " in Termin " << Termine[t]->getident()
     << endl;
#endif
				Termine[t]->aktuell++;
			}
		}
		s->checkdouble();
	}
	
	saerger = 0;
	gaerger = 0;
	foreach (j, Student)
		saerger += Studenten[j]->aerger();
	foreach (j, Termin)
		gaerger += Termine[j]->aerger();
	aerger = saerger + gaerger;
}

void printverteilung(num temp)
{	int vv, t;
	Vorlesung *v;

	foreach(vv, Vorlesung)
	{
		v = Vorlesungen[vv];
		cerr << v->getident() << "> ";
		for (t=0; t<v->termine; t++)
		{
			cerr << Termine[v->termin[t]]->getident()
			     << ":"
			     << Termine[v->termin[t]]->aktuell << " ";
		}
		cerr << endl;
	}
	cerr << "Temp " << temp;
	cerr << "  Gesammt„rger " << aerger
	     << "  Gruppen„rger " << gaerger
	     << "  Studenten„rger " << saerger
	     << endl;
}


void check_termine()
{	int v, t;

// Hat jede Vorlesung Termine?
	foreach(v, Vorlesung)
	{
		if (Vorlesungen[v]->termine == 0)
		{
			cerr << "+++ Vorlesung "
			     << Vorlesungen[v]->getident()
			     << " hat keine Termine."
			     << endl;
			errors++;
		}
	}
	
// Ist jeder Termin einer Vorlesung zugeordnet?
	foreach(t, Termin)
	{
		if (Termine[t]->vorlesungsindex < 0)
		{
			cerr << "+++ Termin "
			     << Termine[t]->getident()
			     << " ist keine Vorlesung zugeordnet."
			     << endl;
			errors++;
		}
	}
}

void abkuehlen(num temp)
{	int n;
	Student *s;
	
	for(n=0; n<global.zyklen; n++)
	{
		diffaerger=0;
		s = Studenten[Student::einStudent()];
		s->wandere();
#if WANDER
cerr << " " << temp * (rand() & 0x7fff) / 0x8000;
#endif
		if (diffaerger > temp * (rand() & 0x7fff) / 0x8000)
		{
			s->undo();
#if WANDER
cerr << " ***U***";
#endif
		}
#if WANDER
cerr << endl;
#endif
	}
}


ostream& operator <<(ostream &os, Student &s)
{	int v;

	os << s.name;
	foreach(v, Vorlesung)
	{
		if (s.intermin[v] >= 0)
		{
			os << '#' << Vorlesungen[v]->getident()
			   << Termine[s.intermin[v]]->getident()
			   << ' ';
		}
	}
	return os;
}
		
void zusammenfassung(const char *name, num arg)
{	ofstream out(name);
	int vv, tt, ss;
	Vorlesung *v;
	Termin *t;
	Student *s;
	
	if (!out)
	{
		perror(name);
		exit(1);
	}
	
	out << "------------------------------------------------------------------------------" << endl
	    << "Zusammenfassung" << endl
	    << "------------------------------------------------------------------------------" << endl;
	foreach(vv, Vorlesung)
	{
		v = Vorlesungen[vv];
		out << v->getname() << " (Other " << v->other << " Penaltys";
		for(tt=0; tt<v->penaltys; tt++)
			out << ' ' << v->penalty[tt];
		out << ")" << endl;
		for (tt=0; tt<v->termine; tt++)
		{
			t = Termine[v->termin[tt]];
			out << t->getident() << ": ";
			out.width(3);
			out << t->aktuell;
			out.width(0);
			out << " Stud, Opt "
			    << t->opt
			    << " Min " << t->minimum
			    << " Max " << t->maximum
			    << " Penalty " << t->penalty
			    << "  Name " << t->getname()
			    << endl;
		}
		out << endl;
	}
	out << "Gesammt„rger " << aerger
	    << "  Gruppen„rger " << gaerger
	    << "  Studenten„rger " << saerger
	    << endl
	    << "Temperatur " << global.temperatur
	    << "  Abkhlunkskoeffizient " << global.z << '/' << global.n
	    << " bei " << global.zyklen << " Zyklen."
	    << endl
	    << "Maxpenalty " << global.penaltyformaximum
	    << "  Minpenalty " << global.penaltyforminimum
	    << endl << endl;
	if (!arg)
		return;
	out << "------------------------------------------------------------------------------" << endl
	    << "Studenten mit Žrger >= " << arg << endl
	    << "------------------------------------------------------------------------------" << endl;

	foreach(ss, Student)
	{
		s = Studenten[ss];
		if (s->aerger() >= arg)
		{
			out.width(4);
			out << s->aerger();
			out.width(0);
			out << ": " << *s << endl;
		}
	}
}

void ausgabe()
{	int s;
	
	foreach(s, Student)
		cout << *Studenten[s] << endl;
}
	

int main(int argc, char **argv)
{	num temp;

	global.temperatur = 50000;
	global.zyklen = 2000;
	global.z = 15;
	global.n = 16;
	global.penaltyforminimum = 5000;
	global.penaltyformaximum = 5000;
	
	if (argc<2)
	{
		cerr << "This is "__FILE__" Version "VERSION" compiled "__DATE__" by Martin Lercher" << endl;
		cerr << "usage: " << *argv << " terminfile [namefile [zsffile mit„rger]] [>ergebnisfile]" << endl;
		return 1;
	}
	cerr << "Phase 1 (Lesen der Termin- und Vorlesungsdatei) beginnt ..." << endl;
	lies_termine(argv[1]);
	if (argc==2)
		print_termine();
	check_termine();
	if (errors)
	{
		cerr << "+++ " << errors << " Fehler. Programm wird abgebrochen." << endl;
		exit(2);
	}
	if (argc==2)
		return errors>0;
		
	cerr << "Phase 2 (Lesen der Namensdatei) beginnt ..." << endl;
	lies_studenten(argv[2]);
	if (errors)
	{
		cerr << "+++ " << errors << " Fehler. Programm wird abgebrochen." << endl;
		exit(2);
	}
	randomize();
	initialverteilung();

	temp = global.temperatur;
	while(temp)
	{
		abkuehlen(temp);
		temp *= global.z;
		temp /= global.n;
		printverteilung(temp);
	}
	if (argc==5)
	{
		cerr << "Phase 3 (Zusammmenfassung) beginnt ..." << endl;
		zusammenfassung(argv[3], atol(argv[4]));
	}
	cerr << "Phase 4 (Ausgabe) beginnt ..." << endl;
	ausgabe();
	return 0;
}
