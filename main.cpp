//
//  main.cpp
//  Hackable
//
//  Created by Kamil Górzyński on 03/01/16.
//  Copyright © 2016 Kamil Górzyński.
//

#include "stdafx.h"
#include <Urlmon.h>
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <WbemCli.h>
#include <string>
#include <queue>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Urlmon")

using namespace std;

typedef pair<int, pair<string, string> > modelzproducentem;
typedef pair<bool, long long> paranapamiec;

queue<int>typpamieci, taktowaniepamieci;
queue<long long int>pojemnoscpamieci;
fstream logg;

HRESULT hRes;
IWbemLocator* pLocator;
IWbemServices* pService;
IEnumWbemClassObject* pEnumerator;

wchar_t procesor[] = L"SELECT * FROM Win32_Processor";
wchar_t procesor_rubryka[] = L"Name";
wchar_t procesor_opis[] = L"Procesor: ";

wchar_t virtualization[] = L"SELECT * FROM Win32_Processor";
wchar_t virtualization_rubryka[] = L"VirtualizationFirmwareEnabled";
wchar_t virtualization_opis[] = L"Obsluga wirtualizacji: ";

wchar_t pamiec0[] = L"SELECT * FROM Win32_PhysicalMemory";
wchar_t pamiec0_rubryka[] = L"BankLabel";
wchar_t pamiec0_opis[] = L"Bank: ";

wchar_t pamiec1[] = L"SELECT * FROM Win32_PhysicalMemory";
wchar_t pamiec1_rubryka[] = L"Capacity"; 
wchar_t pamiec1_opis[] = L"Dostepna pamiec (bajty): ";

wchar_t pamiec2[] = L"SELECT * FROM Win32_PhysicalMemory";
wchar_t pamiec2_rubryka[] = L"MemoryType";
wchar_t pamiec2_opis[] = L"Typ pamieci: ";

wchar_t pamiec3[] = L"SELECT * FROM Win32_PhysicalMemory";
wchar_t pamiec3_rubryka[] = L"Speed";
wchar_t pamiec3_opis[] = L"Taktowanie: ";

wchar_t plytagl[] = L"SELECT * FROM Win32_BaseBoard";
wchar_t plytagl_rubryka[] = L"Manufacturer";
wchar_t plytagl_opis[] = L"Plyta glowna: ";

wchar_t plytamodel[] = L"SELECT * FROM Win32_BaseBoard";
wchar_t plytamodel_rubryka[] = L"Product";
wchar_t plytamodel_opis[] = L"Model plyty: ";

wchar_t siec[] = L"SELECT * FROM Win32_NetworkAdapter";
wchar_t siec_rubryka[] = L"PNPDeviceID";
wchar_t siec_opis[] = L"Karta sieciowa: ";

wchar_t dzwiek[] = L"SELECT * FROM Win32_SoundDevice";
wchar_t dzwiek_rubryka[] = L"DeviceID";
wchar_t dzwiek_opis[] = L"Karta dzwiekowa: ";

wchar_t grafika[] = L"SELECT * FROM Win32_VideoController";
wchar_t grafika_rubryka[] = L"Description";
wchar_t grafika_opis[] = L"Karta graficzna: ";

int rozpocznij()
{
	hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hRes))
	{
		cout << "Nie mozna zainicjalizowac COM: 0x" << std::hex << hRes << endl;
		return 1;
	}

	if ((FAILED(hRes = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0))))
	{
		cout << "Nie mozna zainicjalizowac security: 0x" << std::hex << hRes << endl;
		return 1;
	}

	pLocator = NULL;
	if (FAILED(hRes = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pLocator))))
	{
		cout << "Nie mozna utworzyc WbemLocator: " << std::hex << hRes << endl;
		return 1;
	}

	pService = NULL;
	if (FAILED(hRes = pLocator->ConnectServer(L"root\\CIMV2", NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &pService)))
	{
		pLocator->Release();
		cout << "Nie mozna polaczyc sie z \"CIMV2\": " << std::hex << hRes << endl;
		return 1;
	}
	return 0;
}

ifstream::pos_type filesize(const char* filename)
{
	ifstream in(filename, ifstream::ate | ifstream::binary);
	return in.tellg();
}

bool is_file_exist(const char *fileName)
{
	ifstream infile(fileName);
	return infile.good();
}

int aktualizujbaze(char* baza, LPCWSTR bazastring, LPCWSTR link)
{
	if(URLDownloadToFile(NULL, link, L"new.txt", 0, NULL) != S_OK) return 1;
	fstream stary, nowy, zeszkaluj;
	zeszkaluj.open("new.txt", ios::app);
	zeszkaluj << "jp2gmd";
	zeszkaluj.close();
	long long nowyrozm = filesize("new.txt");
	long long staryrozm = filesize(baza);
	if ((nowyrozm <= staryrozm && (staryrozm>0)))return 0;
	CopyFile(L"new.txt", bazastring, FALSE);
	return 2;
}

void aktualizujdialog()
{
	cout << "Szukam aktualizacji bazy podzespolow\n";
	int wynik1 = aktualizujbaze("baza1.txt", L"baza1.txt", L"http://pcidatabase.com/reports.php?type=csv");
	int wynik2 = aktualizujbaze("baza2.txt", L"baza2.txt", L"http://pci-ids.ucw.cz/v2.2/pci.ids");
	if ((wynik1 == 2) | (wynik2 == 2)) cout << "Zaktualizowano baze podzespolow\n";
	else if ((wynik1 == 0)&&(wynik2==0)) cout << "Baza podzespolow jest aktualna\n";
	else cout << "Blad aktualizacji bazy podzespolow\n";
	cout << "\n";
}

int brakdanych()
{
	pLocator->Release();
	pService->Release();
	cout << "Blad pobierania danych o podzespole: " << std::hex << hRes << endl;
	return 1;
}

modelzproducentem szukajpcidatabase(string producent, string model)
{
	fstream plik;
	plik.open("baza1.txt", ios::in);
	string dane, nazwaproducenta, nazwaproducentaprior, nazwamodelu;
	if (plik.good() == true)while(dane != "jp2gmd")
	{
		getline(plik, dane);
		if (dane.compare(3, 4, producent) == 0 && dane.compare(12, 4, model) == 0) // Pobierz nazwÍ modelu i producenta
		{
			nazwaproducentaprior.clear();
			nazwamodelu.clear();
			int cudzyslowcount = 0;
			int akcje = 0;
			for (unsigned long i = 0; i<dane.length(); ++i)
			{
				if (dane[i] == '"')cudzyslowcount++;
				if (cudzyslowcount == 5 && dane[i] != '"')
				{
					if (akcje == 0)akcje++;
					nazwaproducentaprior += dane[i];
				}
				if (cudzyslowcount == 6 && akcje == 1)akcje++;
				if (cudzyslowcount == 7 && dane[i] != '"')nazwamodelu += dane[i];
			}
		}

		if (dane.compare(3, 4, producent) == 0)  // Pobierz nazwÍ producenta (model nieznany)
		{
			nazwaproducenta.clear();
			int cudzyslowcount = 0;
			int akcje = 0;
			for (unsigned long i = 0; i < dane.length(); ++i)
			{
				if (dane[i] == '"')cudzyslowcount++;
				if (cudzyslowcount == 5 && dane[i] != '"')nazwaproducenta += dane[i];
			}
		}
	}
	if (nazwaproducentaprior.length() != 0)return modelzproducentem(0, make_pair(nazwaproducentaprior, nazwamodelu)); // Znaleziono model i producenta
	else if (nazwaproducenta.length() != 0)return modelzproducentem(1, make_pair(nazwaproducenta, model)); // Znaleziono tylko producenta
	else return modelzproducentem(2, make_pair(producent, model));	// Nie znaleziono w bazie
}

modelzproducentem szukajwczechach(string producent, string model)
{
	for (unsigned i = 0; i<producent.length(); ++i)if (producent[i] > 64 && producent[i] < 97)producent[i] += 32; // wielkie na male litery
	for (unsigned i = 0; i<model.length(); ++i)if (model[i] > 64 && model[i] < 97)model[i] += 32; // wielkie na male litery

	//cout << "szukam " << producent << " model:  " << model << endl;
	
	fstream plik;
	bool czyproducent = false, czybylproducent = false, czybylmodel = false;
	plik.open("baza2.txt", ios::in);
	string dane, nazwaproducenta, nazwamodelu;
	if (plik.good() == true)while (dane != "jp2gmd")
	{
		getline(plik, dane);
		if(dane.length() > 3 && dane[0]!='#' && dane[0] != '	')
		{
			if (dane.compare(0, 4, producent) == 0)
			{
				nazwaproducenta = dane.substr(6);
				czyproducent = true;
				czybylproducent = true;
			}
			else czyproducent = false;
		}

		if (czyproducent && dane.compare(1, 4, model) == 0)
		{
			czybylmodel = true;
			nazwamodelu = dane.substr(7);
		}
	}
	//cout << "pisze " << nazwaproducenta << "   " << nazwamodelu << endl;
	if(czybylproducent && czybylmodel)return modelzproducentem(0, make_pair(nazwaproducenta, nazwamodelu));
	else if(czybylproducent)return modelzproducentem(1, make_pair(nazwaproducenta, model));
	return modelzproducentem(2, make_pair(producent, model));
}

void szukaj(string producent, string model, string typkarty)
{
	cout << typkarty;
	logg << typkarty;
	modelzproducentem wynikpcidatabase = szukajpcidatabase(producent, model);
	modelzproducentem wynikczeski = szukajwczechach(producent, model);
	modelzproducentem wynik;
	if(wynikczeski.first<wynikpcidatabase.first)wynik = wynikpcidatabase;
	else wynik = wynikczeski;

	if (wynik.first == 0)cout << "Producent: " << wynik.second.first << "   Model: " << wynik.second.second << endl << endl;
	else if(wynik.first == 1)cout << "Producent: " << wynik.second.first << "   ID modelu: 0x" << model << endl << endl;
	else cout << "ID producenta: 0x" << producent << "   ID modelu: 0x" << model << endl << endl;

	if (wynik.first == 0)logg << "Producent: " << wynik.second.first << "   Model: " << wynik.second.second << endl << endl;
	else if (wynik.first == 1)logg << "Producent: " << wynik.second.first << "   ID modelu: 0x" << model << endl << endl;
	else logg << "ID producenta: 0x" << producent << "   ID modelu: 0x" << model << endl << endl;
}
  
void podajsiec(std::wstring ciag)
{
	if (ciag.compare(0, 3, L"PCI") != 0 && ciag.compare(0, 3, L"USB") != 0)return;
	wstring producent = ciag.substr(8, 4);
	wstring model = ciag.substr(17, 4);
	string prod(producent.begin(), producent.end());
	string mode(model.begin(), model.end());
	if (ciag.compare(0, 3, L"PCI") == 0) szukaj(prod, mode, "Karta sieciowa PCI\n");
	else szukaj(prod, mode, "Karta sieciowa USB\n");
}

void podajdzwiek(std::wstring ciag)
{
	if (ciag.compare(0, 7, L"HDAUDIO") != 0)return;
	wstring producent = ciag.substr(20, 4);
	wstring model = ciag.substr(29, 4);
	string prod(producent.begin(), producent.end());
	string mode(model.begin(), model.end());
	szukaj(prod, mode, "Karta dzwiekowa HD Audio\n");
}

void przetworzstringa(int typ, BSTR bs, wchar_t opis[])
{
	wstring ciag(bs, SysStringLen(bs));
	string bss(ciag.begin(), ciag.end() );
	wstring ciag2(opis);
	string s(ciag2.begin(), ciag2.end());
	if (typ == 1)podajsiec(ciag);
	else if (typ == 2)podajdzwiek(ciag);
	else if (typ == 11)pojemnoscpamieci.push( _wtoi64(bs) );
	else
	{
		cout << s << bss << endl;
		logg << s << bss << endl;
	}		
}

void przetworzinta(int typ, int liczba, wchar_t opis[])
{
	wstring ciag(opis);
	string s(ciag.begin(), ciag.end());
	if (typ == 12)typpamieci.push(liczba);
	else if (typ == 13)taktowaniepamieci.push(liczba);
	else
	{
		cout << s << liczba << endl;
		logg << s << liczba << endl;
	}		
}

int wyciagnijdane(int typ, wchar_t podzespol[], wchar_t rubryka[], wchar_t opis[])
{
	wstring ciag(opis);
	string s(ciag.begin(), ciag.end());

	pEnumerator = NULL;
	if (FAILED(hRes = pService->ExecQuery(L"WQL", podzespol, WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumerator))) return brakdanych();
	IWbemClassObject* clsObj = NULL;
	int numElems;
	while ((hRes = pEnumerator->Next(WBEM_INFINITE, 1, &clsObj, (ULONG*)&numElems)) != WBEM_S_FALSE)
	{
		if (FAILED(hRes))break;
		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(rubryka, 0, &vRet, NULL, NULL)) && vRet.vt == VT_BOOL)
		{
			if(vRet.boolVal)cout << s << "AKTYWNA" << endl;
			else cout << s << "NIEAKTYWNA" << endl;

			if (vRet.boolVal)logg << s << "AKTYWNA" << endl;
			else logg << s << "NIEAKTYWNA" << endl;
			VariantClear(&vRet);
		}
		if (SUCCEEDED(clsObj->Get(rubryka, 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			przetworzstringa(typ, vRet.bstrVal, opis);
			VariantClear(&vRet);
		}
		if (SUCCEEDED(clsObj->Get(rubryka, 0, &vRet, NULL, NULL)) && vRet.vt == 3)
		{
			przetworzinta(typ, vRet.intVal, opis);
			VariantClear(&vRet);
		}
		clsObj->Release();
	}
	return 0;
}

paranapamiec pojemnoscsi(long long bajty) //	 0 - MB		;    1 - GB
{
	if (bajty > (1 << 30))
	{
		bajty /= (1 << 29);
		if (bajty % 2 == 0)return make_pair(1, bajty / 2);
		else return make_pair(1, (bajty / 2 + 1) );
	}
	else return make_pair(0, (bajty / (1 << 20) ) );
}

string wypisztyppamieci(int typ)
{
	if (typ == 20)return "DDR  ";
	else if (typ == 21)return "DDR2  ";
	else if (typ == 22)return "DDR2 FB - DIMM  ";
	else if (typ == 24)return "DDR3  ";
	else return "Typ nieznany  ";
}

void wypiszpamiec()
{
	cout << "Pamiec RAM: \n";
	logg << "Pamiec RAM: \n";
	int liczkosci = 0;
	while (!pojemnoscpamieci.empty())
	{
		liczkosci++;
		cout << "Slot " << liczkosci << ": ";
		logg << "Slot " << liczkosci << ": ";
		paranapamiec capacity = pojemnoscsi(pojemnoscpamieci.front());
		cout << capacity.second;
		logg << capacity.second;
		if (capacity.first)cout << " GB ";
		else cout << " MB ";
		if (capacity.first)logg << " GB ";
		else logg << " MB ";
		pojemnoscpamieci.pop();
		cout << wypisztyppamieci(typpamieci.front());
		logg << wypisztyppamieci(typpamieci.front());
		typpamieci.pop();
		cout << taktowaniepamieci.front() << "MHz\n";
		logg << taktowaniepamieci.front() << "MHz\n";
		taktowaniepamieci.pop();
	}
	cout << "\n";
	logg << "\n";
}

void debuguj()
{
	if (wyciagnijdane(0, procesor, procesor_rubryka, procesor_opis) != 0)cout << "Blad pobierania danych o procesorze\n";
	if (wyciagnijdane(0, virtualization, virtualization_rubryka, virtualization_opis) != 0)cout << "Blad pobierania danych o obsludze wirtualizacji\n";
	if (wyciagnijdane(0, pamiec0, pamiec0_rubryka, pamiec0_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	if (wyciagnijdane(0, pamiec1, pamiec1_rubryka, pamiec1_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	if (wyciagnijdane(0, pamiec2, pamiec2_rubryka, pamiec2_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	if (wyciagnijdane(0, pamiec3, pamiec3_rubryka, pamiec3_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	if (wyciagnijdane(0, plytagl, plytagl_rubryka, plytagl_opis) != 0)cout << "Blad pobierania danych o plycie glownej\n";
	if (wyciagnijdane(0, plytamodel, plytamodel_rubryka, plytamodel_opis) != 0)cout << "Blad pobierania danych o plycie glownej\n";
	if (wyciagnijdane(0, siec, siec_rubryka, siec_opis) != 0)cout << "Blad pobierania danych o karcie sieciowej\n";
	if (wyciagnijdane(0, dzwiek, dzwiek_rubryka, dzwiek_opis) != 0)cout << "Blad pobierania danych o karcie dzwiekowej\n";
	if (wyciagnijdane(0, grafika, grafika_rubryka, grafika_opis) != 0)cout << "Blad pobierania danych o karcie graficznej\n";
}

void loguj()
{
	logg.open("log.txt", ios::out | std::ofstream::trunc);
	long long rozmiarbazy = filesize("baza.txt");
	if ((rozmiarbazy == 0) | (!is_file_exist("baza.txt")))aktualizujdialog();
	if (wyciagnijdane(0, procesor, procesor_rubryka, procesor_opis) != 0)cout << "Blad pobierania danych o procesorze\n";
	if (wyciagnijdane(0, virtualization, virtualization_rubryka, virtualization_opis) != 0)cout << "Blad pobierania danych o obsludze wirtualizacji\n";
	cout << "\n";
	logg << "\n";
	if (wyciagnijdane(11, pamiec1, pamiec1_rubryka, pamiec1_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	if (wyciagnijdane(12, pamiec2, pamiec2_rubryka, pamiec2_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	if (wyciagnijdane(13, pamiec3, pamiec3_rubryka, pamiec3_opis) != 0)cout << "Blad pobierania danych o pamieci\n";
	wypiszpamiec();
	if (wyciagnijdane(0, plytagl, plytagl_rubryka, plytagl_opis) != 0)cout << "Blad pobierania danych o plycie glownej\n";
	if (wyciagnijdane(0, plytamodel, plytamodel_rubryka, plytamodel_opis) != 0)cout << "Blad pobierania danych o plycie glownej\n";
	cout << "\n";
	logg << "\n";
	if (wyciagnijdane(1, siec, siec_rubryka, siec_opis) != 0)cout << "Blad pobierania danych o karcie sieciowej\n";
	cout << "\n";
	logg << "\n";
	if (wyciagnijdane(2, dzwiek, dzwiek_rubryka, dzwiek_opis) != 0)cout << "Blad pobierania danych o karcie dzwiekowej\n";
	cout << "\n";
	logg << "\n";
	if (wyciagnijdane(0, grafika, grafika_rubryka, grafika_opis) != 0)cout << "Blad pobierania danych o karcie graficznej\n";
	cout << "\n";
	logg << "\n";
	logg << "Wygenerowano za pomocą Hackable\n©Hackintosh Polska 2016\nhttps://facebook.com/groups/hackintoshpolska/";
	remove("new.txt");
}

void credits()
{
	cout << "\nTworcy aplikacji: \nProgramista: Kamil Gorzynski\nTester: Kamil Kasperski\nBazy danych:	http://pcidatabase.com/	  http://pci-ids.ucw.cz\nHackintosh Polska 2016\n\n";
}

bool dialogkoncowy()
{
	int codalej;
	cout << endl << "1 - wyswietl liste podzespolow" << endl << "2 - aktualizuj baze podzespolow" << endl << "3 - debuguj" << endl << "4 - tworcy aplikacji" << endl << "0 - wyjscie" << endl;
	cin >> codalej;
	if (codalej == 0)return 0;
	else if (codalej == 1)loguj();
	else if (codalej == 2)aktualizujdialog();
	else if (codalej == 3)debuguj();
	else if (codalej == 4)credits();
	dialogkoncowy();
	return 0;
}

int main()
{
	cout << "Hackable by Hackintosh Polska" << endl << "Poznaj specyfikacje swojego komputera" << endl << endl;
	if (rozpocznij() != 0)cout << "Blad polaczenia z baza danych\n";
	loguj();
	dialogkoncowy();
	pEnumerator->Release();
	pService->Release();
	pLocator->Release();
	return 0;
} 
