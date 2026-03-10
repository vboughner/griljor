/* header file for functions used in Burt program
   Van Boughner   May 1989   */

#define KEY_LENGTH		40
#define PRONOUN_LENGTH		15
#define LINE_LENGTH		250
#define MAX_PERSONALITIES	1000	/* maximum different personalities */

#define FALSE	0
#define TRUE	1


struct keywords {
  char             word[KEY_LENGTH];
  int              count;
  struct keywords  *next;
} ;


struct response_lines {
  char 			*line;
  int			count;
  struct response_lines *next;
} ;


enum key_types { standard, question, substitution };

struct keyword_records {
  struct keywords	  *keyword_list;
  struct response_lines	  *response_list;
  struct response_lines	  *last_response;
  int			  num_of_lines;
  int                     count;
  enum key_types	  type;
  struct keyword_records  *next;
} ;


struct pronoun_subs {
  char s1[PRONOUN_LENGTH];
  char s2[PRONOUN_LENGTH];
  struct pronoun_subs *next;
} ;


struct personalities {
  struct keyword_records	*sub_list;
  struct keyword_records	*history_list;
  struct keyword_records	*main_list;
  struct pronoun_subs		*pronoun_list;
} ;


/* headers for procedures */
char *get_input(), *make_response();
struct personalities *allocate_personality();
