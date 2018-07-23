export class SearchUtil {
    public static LAST_SEARCH_DATA = 'last_search_data';

    public static EVENT_KEY_ENETER = 13;
    public static EVENT_KEY_LEFT = 37;
    public static EVENT_KEY_RIGHT = 39;
    public static EVENT_KEY_UP = 38;
    public static EVENT_KEY_DOWN =  40;
    public static EVENT_KEY_TAB = 9;

  // match src/github.com/pensando/sw/venice/ui/venice-sdk/v1/models/generated/search/enums.ts FieldsRequirement_operator
  public static SEARCH_FIELD_OPERATORS = [
    { 'operator': '>=', 'label': 'gte' , 'description': 'greater than or equal'},
    { 'operator': '<=', 'label': 'less and equal' , 'description': 'less than or equal' },
    { 'operator': '=', 'label': 'equals' , 'description': 'equals' },
    { 'operator': '==', 'label': 'equals' , 'description': 'equals' },
    { 'operator': '!=', 'label': 'not equals'  , 'description': 'not equals'},
    { 'operator': '>', 'label': 'greater  than' , 'description': 'greater than' },
    { 'operator': '<', 'label': 'less than' , 'description': 'less than' }
  ];

  // match src/github.com/pensando/sw/venice/ui/venice-sdk/v1/models/generated/search/enums.ts LabelsRequirement_operator
  public static SEARCH_LABEL_OPERATORS = [
    { 'operator': '==', 'label': 'equals' , 'description': 'equals' },
    { 'operator': '=', 'label': 'equals' , 'description': 'equals' },
    { 'operator': '!=', 'label': 'not equals' , 'description': ' not equals' },
    { 'operator': '=~', 'label': 'in' , 'description': 'contains'  },
    { 'operator': '!~', 'label': 'notIn' , 'description': 'not contains'  }
  ];

  public static SEARCH_GRAMMAR_TAGS = {
    'in': {
      'key': 'in:',
      'content': 'category'
    },
    'is': {
      'key': 'is:',
      'content': 'kind'
    },
    'has': {
      'key': 'has:',
      'content': 'field'
    },
    'tag': {
      'key': 'tag:',
      'content': 'label'
    }
  };
}
