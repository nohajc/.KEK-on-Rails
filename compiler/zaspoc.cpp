/* zaspoc.cpp */

#include "zaspoc.h"
#include "tabsym.h"
#include <stdio.h>

struct Instr {
	TypInstr typ;
	int opd;
	Label lab;
};

enum {
	MaxZas = 100, MaxProm = 100, MaxProg = 200
};

static int z[MaxZas];          // zasobnik
static int v;                  // vrchol zasobniku
static int m[MaxProm];         // pamet promennych
static Instr p[MaxProg];       // pamet programu;
static int ic;                 // citac instrukci

int Gener(TypInstr ti, int opd, Label lab) {
	p[ic].typ = ti;
	p[ic].opd = opd;
	p[ic].lab = lab;
	return ic++;
}

void resolveBreak(int a1, int a2) {
	for (int i = a1; i < a2; ++i) {
		if (p[i].lab == UBRK) {
			PutIC(i);
			p[i].lab = BRK;
		}
	}
}

/*int Gener(TypInstr ti, int opd, Label lab) {
	p[ic].typ = ti;
	p[ic].opd = opd;
	p[ic].lab = lab;
	return ic++;
}*/

void GenTR(char *id) {
	int v;
	DruhId druh = idPromKonst(id, &v);
	switch (druh) {
	case IdProm:
		Gener(TA, v);
		Gener(DR);
		break;
	case IdKonst:
		Gener(TC, v);
		break;
	default:
		return;
	}
}

void PutIC(int adr) {
	p[adr].opd = ic;
}

int GetIC() {
	return ic;
}

void Print() {
	int ic = 0;
	printf("\nVypis programu\n");
	for (;;) {
		printf("%3d: ", ic);
		switch (p[ic].typ) {
		case TA:
			printf("TA  %d\n", p[ic].opd);
			break;
		case TC:
			printf("TC  %d\n", p[ic].opd);
			break;
		case BOP:
			printf("BOP %d\n", p[ic].opd);
			break;
		case UNM:
			printf("UNM\n");
			break;
		case DR:
			printf("DR\n");
			break;
		case ST:
			printf("ST\n");
			break;
		case IFJ:
			printf("IFJ %d\n", p[ic].opd);
			break;
		case JU:
			printf("JU  %d\n", p[ic].opd);
			break;
		case WRT:
			printf("WRT\n");
			break;
		case RD:
			printf("RD %d\n", p[ic].opd);
			break;
		case DUP:
			printf("DUP\n");
			break;
		case SWAP:
			printf("SWAP\n");
			break;
		case NOT:
			printf("NOT\n");
			break;
		case STOP:
			printf("STOP\n\n");
			return;
		}
		ic++;
	}
}

void Run() {
	Instr instr;
	printf("\nInterpretace programu\n");
	ic = 0;
	v = -1;
	for (;;) {
		instr = p[ic++];
		switch (instr.typ) {
		case TA:
			z[++v] = instr.opd;
			break;
		case TC:
			z[++v] = instr.opd;
			break;
		case BOP: {
			int right = z[v--];
			int left = z[v--];
			switch (instr.opd) {
			case Plus:
				z[++v] = left + right;
				break;
			case Minus:
				z[++v] = left - right;
				break;
			case Times:
				z[++v] = left * right;
				break;
			case Divide:
				z[++v] = left / right;
				break;
			case Modulo:
				z[++v] = left % right;
				break;
			case Eq:
				z[++v] = left == right;
				break;
			case NotEq:
				z[++v] = left != right;
				break;
			case Less:
				z[++v] = left < right;
				break;
			case Greater:
				z[++v] = left > right;
				break;
			case LessOrEq:
				z[++v] = left <= right;
				break;
			case GreaterOrEq:
				z[++v] = left >= right;
				break;
			case LogOr:
				z[++v] = left || right;
				break;
			case LogAnd:
				z[++v] = left && right;
				break;
			case BitOr:
				z[++v] = left | right;
				break;
			case BitAnd:
				z[++v] = left & right;
				break;
			case Xor:
				z[++v] = left ^ right;
				break;
			case Lsh:
				z[++v] = left << right;
				break;
			case Rsh:
				z[++v] = left >> right;
				break;
			}
			break;
		}
		case UNM:
			z[v] = -z[v];
			break;
		case NOT:
			z[v] = !z[v];
			break;
		case DR:
			z[v] = m[z[v]];
			break;
		case ST: {
			int val = z[v--];
			int adr = z[v--];
			m[adr] = val;
			break;
		}
		case IFJ:
			if (!z[v--])
				ic = instr.opd;
			break;
		case JU:
			ic = instr.opd;
			break;
		case WRT:
			printf("%d\n", z[v--]);
			break;
		case RD: {
			int res = scanf("%d", &m[z[v--]]);
			if(res != 1) {
				printf("I/O Error.\n\n");
				return;
			}
			break;
		}
		case DUP:
			z[v + 1] = z[v];
			v++;
			break;
		case SWAP: {
			int tmp = z[v - 1];
			z[v - 1] = z[v];
			z[v] = tmp;
			break;
		}
		case STOP:
			printf("Konec interpretace\n\n");
			return;
		}
	}
}

