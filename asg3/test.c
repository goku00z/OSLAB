#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void clearTabs(char *line){
	// removes spaces and tabs from line
	// like "   \t  bbbb   \t\t  rrrr \t  ggg\t  "--->"bbbb rrrr ggg"
	int i,j;
	// convert all tabs into spaces
	// printf("\n*****************BEFORE TAB REMOVAL : '%s'",line);
	i=j=0;
	while(line[i]!='\0'){
		if(line[i]=='\t'||line[i]==9||line[i]==11){
			// printf("\ntab detected !!");
			line[i]=' ';
		}
		i++;
	}
	// printf("\n****************AFTER TAB REMOVAL : '%s'",line);
	// all tabs are replaced by spaces,,,now we will remove redundant spaces[inplace]
	i=j=0;
	while(line[i]==' '){
		i++;
	}
	while(1){
		if(line[i]=='\0'){
			break;
		}
		if(line[i]!=' '){
			line[j]=line[i];
			j++;
			i++;
			continue;
		}
		line[j]=' ';
		i++;
		j++;
		while(line[i]==' '){
			i++;
		}
	}
	if(j-1>=0){
		if(line[j-1]==' '){
			line[j-1]='\0';
		}
	}
	line[j]='\0';
	// printf("\n****************FINAL TAB REMOVAL : '%s'",line);
	return;
}
void removeLastEnter(char *a){
	int i = 0;
	while(1){
		if(a[i] == '\0'){
			break;
		}
		i++;
	}
	if((i-1)=='\n'){
		printf("\njbkjbkjb\n");
		a[i-1] = '\0';
	}
	return;
}

int extractWord(char *line,char x,char *word){
	int i=0;
	int count=0;
	int posx;
	word[0] = '\0';
	while(line[i]!='\0'){
		if(line[i]==x){
			count++;
			posx = i;
		}
		i++;
	}
	if(count>1){
		return -1;
	}
	if(count == 0){
		return 0;
	}
	int len = i;
	i = posx+1;
	int nextWord=0;
	while(i<len){
		if(line[i]=='<'||line[i]=='>'){
			word[0] = '\0';
			return -1;
		}
		if(line[i]!=' '&&line[i]!='\n'&&line[i]!='\t'&&line[i]!='|'){
			nextWord = i;
			break;
		}
		i++;
	}
	if(nextWord == 0){
		return -1;
	}
	// printf("\nnext word found at  = %d",nextWord);
	int j = 0;
	while(1){
		if(line[i]==' '||line[i]=='\n'||line[i]=='\t'||line[i]=='|'||line[i]=='\0'){
			nextWord = i;
			word[j] = '\0';
			break;
		}
		word[j] = line[i];
		j++;
		i++;
	}
	while(i<len){
		line[posx] = line[i];
		i++;
		posx++;
	}
	line[posx] = '\0';
	return 1;
}


int main(){
	const char *a = "sadasdas < > adadas kjjk";
	char b[100];
	strcpy(b,a);

	// scanf("%[^\t\n]",b);

	printf("\nb = {%s}",b);

	removeLastEnter(b);
	clearTabs(b);
	printf("\nb = {%s}",b);
	char word[100];

	int i;
	
	i=extractWord(b,'<',word);
	clearTabs(b);
	printf("\ni = %d line = {%s} input = {%s}",i,b,word);

	i=extractWord(b,'>',word);
	clearTabs(b);
	printf("\ni = %d line = {%s} output = {%s}",i,b,word);

	



	// clearTabs(b);
	// printf("\ni = %d line = {%s} output = {%s}",i,b,word);

	// i=extractWord(b,'<',word);
	// clearTabs(b);
	// printf("\ni = %d line = {%s} input = {%s}",i,b,word);

	printf("\n");
	return 0;
}