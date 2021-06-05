#include<bits/stdc++.h>
using namespace std ;
#define MAXN 1001

string name[MAXN] ;
int credit[MAXN] ;
int score[MAXN] ;
int tried[MAXN] ;
vector<vector<string>> pre[MAXN] ;

int sum_score = 0;
int sum_credit_tried = 0;
int sum_credit_get = 0;
int gratuate_credit = 0 ;

int main() {
	char tmp ;
	int cnt = 1 ;
	scanf("%c", &tmp) ;
	while(1) {
		//printf("TMP = %c\n", tmp) ;
		if ( tmp == '\n' || tmp == '\r')
			break ;

		while ( tmp != '|' ) {
			name[cnt] = name[cnt] + tmp ;
			scanf("%c", &tmp) ;
		}

		// cout << name[cnt] << endl ;

		scanf("%d", &credit[cnt]) ;
		//printf("credit = %d\n", credit[cnt]) ;

		scanf("%c", &tmp) ;
		scanf("%c", &tmp) ;

		while ( tmp != '|') {
			vector<string> cur_pre_set ;
			while ( tmp != ';' && tmp != '|') {
				string cur_pre = "" ;
				while ( tmp != ',' && tmp != ';' && tmp != '|') {
					cur_pre = cur_pre + tmp ;
					scanf("%c", &tmp) ;
					// puts("doing ','") ;
				}
				cur_pre_set.push_back(cur_pre) ;
				if (tmp == ',')
					scanf("%c", &tmp) ;
				// puts("doing ';'") ;
			}
			pre[cnt].push_back(cur_pre_set) ;
			if (tmp == ';')
				scanf("%c", &tmp) ;
			// puts("doing '|'") ;
		}

		scanf("%c", &tmp) ;
		
		//printf("tmp = %c\n ", tmp ) ;

		tried[cnt] = 1 ;
//		if ( tmp == '\n' || tmp == '\r' ) {
//			tried[cnt] = 0 ;
//		} else {
			switch(tmp) {
				case 'A' : 
					score[cnt] = 4 ;
					scanf("%c", &tmp) ;
					break ;
				case'B' :
					score[cnt] = 3 ;
					scanf("%c", &tmp) ;
					break ;
				case'C' :
					score[cnt] = 2 ;
					scanf("%c", &tmp) ;
					break ;
				case'D' :
					score[cnt] = 1 ;
					scanf("%c", &tmp) ;
					break ;
				case'F' :
					score[cnt] = 0 ;
					scanf("%c", &tmp) ;
					break ;
				default:
					tried[cnt] = 0 ;
					score[cnt] = 0 ;
			}
//		}

		gratuate_credit += credit[cnt] ;
		if (tried[cnt]) {
			sum_credit_tried += credit[cnt] ;
			sum_score += score[cnt] * credit[cnt];
			if (score[cnt] > 0) {
				sum_credit_get += credit[cnt] ;
			}
		}
		// cout << "tried = " << tried[cnt] <<endl ;
		// cout << "score = " << score[cnt] << endl ;
		
		scanf("%c", &tmp) ;
		++cnt ;

	}

	cnt = cnt - 1 ;

		/*
	for (int i = 1; i <= cnt; ++i ) {
		cout << name[i] << endl ;
		cout << credit[i] << endl ;
		cout << "pre:\n" ;
		for ( int j = 0; j < pre[i].size(); ++j ) {
			printf("\n\tset:\n") ;
			for ( int k = 0; k < pre[i][j].size(); ++k ) {
				cout << "\t\t" << pre[i][j][k] << endl ;
			}
		}
		cout << "end\n" ;
	}
		*/

	if ( sum_credit_tried == 0 )
		printf("GPA: 0.0\n") ;
	else
		printf("GPA: %.1f\n", 1.0 * sum_score / sum_credit_tried) ;
	printf("Hours Attempted: %d\n", sum_credit_tried) ;
	printf("Hours Completed: %d\n", sum_credit_get) ;
	printf("Credits Remaining: %d\n\n", gratuate_credit - sum_credit_get) ;

	printf("Possible Courses to Take Next\n") ;
	if (gratuate_credit == sum_credit_get ) {
		printf("  None - Congratulations!\n") ;
		return 0 ;
	} else {
		for ( int i = 1; i <= cnt; ++i ) {
			if (!score[i]) {
				// cout << "checking.. " << name[i] << endl ;
				int tag = 0 ;
				if (pre[i].size() == 0)
					tag = 1 ;
				for ( int j = 0; j < pre[i].size(); ++j ) {
					tag = 1 ;
					for ( int k = 0; k < pre[i][j].size(); ++k ) {
						string cur = pre[i][j][k] ;
						// cout << '\t' << cur << endl ;
						int found = 0 ;
						for ( int l = 1; l <= cnt; ++l ) {
							if (name[l] == cur) {
								// cout << "\t\t" << name[l] << endl ;
								found = 1 ;
								if (score[l] == 0)
									tag = 0 ;
								break ;
							}
						}
						if (found == 0)
							tag = 0 ;
					}
					if ( tag == 1 )
						break ;
				}
				if ( tag == 1 ) {
					printf("  ") ;
					cout << name[i] << endl ;
				}
				//getchar() ;
			}
		}
	}


	return 0 ;
}
