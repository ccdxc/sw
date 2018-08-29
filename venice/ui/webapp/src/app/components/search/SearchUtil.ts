import { SearchInputTypeValue, SearchSpec, SearchExpression, SearchModelField } from '@app/components/search';
import { Utility } from '@app/common/Utility';
import { CategoryMapping } from '@sdk/v1/models/generated/category-mapping.model';
import { SearchSuggestion } from './';

export class SearchUtil {
  public static LAST_SEARCH_DATA = 'last_search_data';

  public static EVENT_KEY_ENTER = 13;
  public static EVENT_KEY_LEFT = 37;
  public static EVENT_KEY_RIGHT = 39;
  public static EVENT_KEY_UP = 38;
  public static EVENT_KEY_DOWN =  40;
  public static EVENT_KEY_TAB = 9;

  public static SEARCHFIELD_META = 'meta';
  public static SEARCHFIELD_SPEC = 'spec';
  public static SEARCHFIELD_STATUS = 'status';

  public static META_ATTRIBUTESS = ['name', 'creation-time', 'mod-time', 'resource-version', 'namespace', 'uuid' ];

  public static stringOperators = [
    { label: 'equals', value: 'equals' },
    { label: 'not equals', value: 'not equals' },
    { label: 'in', value: 'in' },
    { label: 'not in', value: 'not in' }
  ];

  public static numberOperators = [
    { label: 'equals', value: 'equals' },
    { label: 'not equals', value: 'not equals' },
    { label: 'greater than', value: 'gt' },
    { label: 'greater than equal', value: 'gte' },
    { label: 'less than', value: 'lt' },
    { label: 'less than equal', value: 'lte' }
  ];

  public static booleanOperators = [
    { label: 'equals', value: 'equals' },
    { label: 'not equals', value: 'not equals' }
  ];

  /**
   *  match src/github.com/pensando/sw/venice/ui/venice-sdk/v1/models/generated/search/enums.ts FieldsRequirement_operator
   *  operator is for use typing in. eg has:meta=~node1
   *  label is for guided-search repeater operator
   *  searchoperator is for search REST API request JSON
   *  description is for tooltip
   */
  public static SEARCH_FIELD_OPERATORS = [
    { 'operator': '>=', 'label': 'gte' , 'description': 'greater than or equal', 'searchoperator': 'gte' },
    { 'operator': '<=', 'label': 'lte' , 'description': 'less than or equal', 'searchoperator': 'lte'  },
    { 'operator': '=', 'label': 'equals' , 'description': 'equal', 'searchoperator': 'equals'  },
    { 'operator': '==', 'label': 'equals' , 'description': 'equal' , 'searchoperator': 'equals' },
    { 'operator': '!=', 'label': 'not equals'  , 'description': 'not equal', 'searchoperator': 'not equals' },
    { 'operator': '>', 'label': 'gt' , 'description': 'greater than', 'searchoperator': 'gt'  },
    { 'operator': '<', 'label': 'lt' , 'description': 'less than', 'searchoperator': 'lt'  },
    { 'operator': '=~', 'label': 'in' , 'description': 'contains', 'searchoperator': 'in'   },
    { 'operator': '!~', 'label': 'not in' , 'description': 'not contains', 'searchoperator': 'not in'   }
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

  public static getSearchInitPrefix(selection: SearchSuggestion): string {
  const type = (selection.name);
    switch (type) {
      case 'category':
       return 'in:';
      case 'kind':
        return 'is:';
      case 'field':
        return 'has:';
      case 'label':
        return 'tag:';
      default:
        return '';
    }
  }

  public static getKinds(): any[] {
    const cats = this.getCategories();
    let kinds = [];
    cats.filter((cat) => {
      const catKeys = Object.keys( CategoryMapping[ cat ] );
      kinds = kinds.concat(catKeys);
    });
    kinds = kinds.sort();
    return kinds;
  }

  public static  getCategories(): any[] {
    let cats =  Object.keys(CategoryMapping);
    cats = cats.sort();
    return cats;
  }

  public static  isValidKind(kind: string): boolean {
    return (SearchUtil.getKinds().indexOf(kind) > -1);
  }

  public static  isValidCategory(kind: string): boolean {
    return (SearchUtil.getCategories().indexOf(kind) > -1);
  }

  /**
   * Follow
   *  pensando/sw/venice/ui/venice-sdk/v1/models/generated/category-mapping.model.ts
   */
  public static getKindsByCategory(selectedCategory: string ): any[] {
    return Object.keys( CategoryMapping[ selectedCategory ] );
  }

  /**
   * Find category from kind.
   * e.g given "Node" as a kind, return "Cluster" as category.
   * see pensando/sw/venice/ui/venice-sdk/v1/models/generated/category-mapping.model.ts
   * @param kind
   */
  public static findCategoryByKind(kind: string ): string {
      const category = null;
      const cats = Object.keys( CategoryMapping);
      for (let i = 0; i < cats.length; i++) {
        const cat = cats[i];
        const kinds = this.getKindsByCategory(cat);
        for (let j = 0; j < kinds.length; j++) {
            if (kind === kinds[j]) {
              return cat;
            }
        }
      }
      return category;
  }

  /**
   * Find Venice-SDK object model informaton
   * @param category
   * @param kind
   */
  public static getModelInfo(category: string,  kind: string): SearchModelField {
    if (! category ) {
      return null;
    }
    const instance = CategoryMapping[category][kind];
    if (!instance) {
      return null;
    }
    const obj: SearchModelField = {
      meta: instance[this.SEARCHFIELD_META] ? Object.keys(instance[this.SEARCHFIELD_META]) : [],
      spec: instance[this.SEARCHFIELD_SPEC] ? Object.keys(instance[this.SEARCHFIELD_SPEC]) : [],
      status: instance[this.SEARCHFIELD_STATUS] ? Object.keys(instance[this.SEARCHFIELD_STATUS]) : [],
    };
    return obj;
  }

  public static getModelInfoByKind(kind: string): any {
    const cat = this.findCategoryByKind(kind);
    return this.getModelInfo(cat, kind);
  }

/**
 * convert string operator to symbol operator
 * @param opStr
 *
 *  if (opStr === 'equals') {
 *     return '=';
 *  }
 */
public static convertSearchSpecOperator(opStr: string): string {
  const operators = SearchUtil.SEARCH_FIELD_OPERATORS ;
  for (let i = 0; i < operators.length; i++) {
    const op = operators[i];
    if (opStr === op.description || opStr === op.label ) {
      return op.operator;
    }
  }
  return '';
}
/**
 * convert symbol operator to string operator
 * @param opStr
 * e.g "="  --> equal
 */
public static convertOperatorString(opStr: string): string {
  const operators = SearchUtil.SEARCH_FIELD_OPERATORS ;
  for (let i = 0; i < operators.length; i++) {
    const op = operators[i];
    if (opStr === op.operator ) {
      return op.label;
    }
  }
  return '';
}




/**
   * This API reformats input string (string middle space,etc ', ' -> ',')
   * It tries to correct user input space error and make the search-input-string grammarically correct.
   *
   * @param value
   */
  public static  formatInputString(value: string): string {
    let inputstr = value.trim(); // "in:cluster is: node  has:meta.name=node9 ";
    inputstr = inputstr.replace(/\s\s+/g, ' ');
    inputstr = inputstr.replace(';', ':'); // format [is;node]  double [is:node]
    inputstr = inputstr.replace(',,', ','); // format [in: cluster is,,monitoring is:node]  double ",,"
    inputstr = inputstr.replace(', ', ',');  // format [in:cluster,  network is:node,tenants]
    inputstr = inputstr.replace(': ', ':');  // format [is:node has: name=node9   in:cluster]
    inputstr = inputstr.replace(' :', ':');  // format [in:cluster is :node,monitoring]
    inputstr = inputstr.replace('= ', '=');  // format [is:node has: name= node9   in:cluster]
    inputstr = inputstr.replace(' =', '=');  // format [is:node has: name =node9   in:cluster]
    inputstr = inputstr.replace(/\s\s+/g, ' ');
    return inputstr;
  }

  /**
   * inputstr as:
   *  in:Cluster is:Node has:name=node9 tag:os=esx,storage=ssd
   * output as:
   *  "[{"type":"in","value":"Cluster"},{"type":"is","value":"Node"},{"type":"has","value":"name=node9"},{"type":"tag","value":"os=esx,storage=ssd"}]"
   * @param inputstr
   */
  public static  parseSearchStringToObjectList(inputstr: string): any[] {
    const list = inputstr.split(' ');
    const outputArray = [];
    let prevObj = null;
    for (let i = 0; list && i < list.length; i++) {
      const listStr = list[i];
      if (listStr && listStr.trim().length > 0) {
        const strs = listStr.split(':');
        if (strs.length === 2) {
          const obj = {};
          obj['type'] = strs[0];
          obj['value'] = strs[1];
          outputArray.push(obj);
          prevObj = obj;
        } else {
          if (prevObj) {
            prevObj['value'] = prevObj['value'] + ' ' + listStr;
          }
        }
      }
    }
    return outputArray;
  }

  /**
   * convert SearchInputObjectList ToSearchSpec
   * list is like
   *  "[{"type":"in","value":"Cluster"},{"type":"is","value":"Node"},{"type":"has","value":"name=node1"},{"type":"tag","value":"os=esx,storage=ssd"}]"
   */
  public static convertToSearchSpec (list: SearchInputTypeValue[]): SearchSpec {
      const searchSpec: SearchSpec  = {};
      for (let i = 0; list && i < list.length; i++) {
        const searchInputTypeValue = list[i];
        const value = searchInputTypeValue.value;
        if (Utility.isEmpty(value)) {
          continue;
        }
        if (!searchSpec[searchInputTypeValue.type]) {
          searchSpec[searchInputTypeValue.type] = value;
         } else {
          searchSpec[searchInputTypeValue.type] = searchSpec[searchInputTypeValue.type] + ',' + value;
         }
      }
      return searchSpec;
  }

  /**
   * Check whether current input search string starts with prammar tag. like "is:xxx"
   */
  public static  isSearchInputStartsWithGrammarTag(inputString: string): boolean {
    return this.isSearchInputStartsEndsWithGrammarTag(inputString, true);
  }

  /**
   * Check whether current input search string end with prammar tag. like "is:xxx has:"
   */
  public static  isSearchInputEndsWithGrammarTag(inputString: string): boolean {
    return this.isSearchInputStartsEndsWithGrammarTag(inputString, false);
  }

  /**
   * Helper function
   * @param inputString
   * @param isStartsWith
   */
  public static  isSearchInputStartsEndsWithGrammarTag(inputString: string, isStartsWith: boolean): boolean {
    inputString = inputString.trim();
    const grammars = Object.keys(SearchUtil.SEARCH_GRAMMAR_TAGS);
    for (let i = 0; i < grammars.length; i++) {
      const op = SearchUtil.SEARCH_GRAMMAR_TAGS[grammars[i]];
      if (isStartsWith) {
        if (inputString.startsWith(op.key)) {
          return true;
        }
      } else {
        if (inputString.endsWith(op.key)) {
          return true;
        }
      }
    }
    return false;
  }

  /**
   * When input search string is like "in:xxx is:yyy has:".  UI builds a object {'start': 'in', 'end':'has'}
   * This object will help compute suggestions and how to update the input-search string.
   * @param inputString
   */
  public static  getFirstLastSearchGrammarTag(inputString: string) {
    const obj = {};
    inputString = inputString.trim();
    const grammars = Object.keys(SearchUtil.SEARCH_GRAMMAR_TAGS);
    for (let i = 0; i < grammars.length; i++) {
      const op = SearchUtil.SEARCH_GRAMMAR_TAGS[grammars[i]];
      if (inputString.startsWith(op.key)) {
        obj['start'] = op.key;
      }
      if (inputString.endsWith(op.key)) {
        obj['end'] = op.key;
      }
    }
    return obj;
  }

  /**
   * parse storage=ssd
   * to:
   * {
   *  key: 'storage',
   *  operator: '=',
   *  values: = ['ssd']
   * }
   */
  public static parseToExpression(inputString: string, isField: boolean): SearchExpression {
    const operators = SearchUtil.SEARCH_FIELD_OPERATORS ;
    const searchExpression: SearchExpression = { key: null, operator: null, values: null};
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (inputString.indexOf(op.operator) >= 0) {
        const strs = inputString.split(op.operator);
        // cases sensitive.
        searchExpression.key = this.padKey(strs[0], isField);
        searchExpression.operator = op.searchoperator;
        searchExpression.values = [strs[1].trim()];
      }
    }
    return searchExpression;
  }

  /**
   * Get operators for guided-search repeater-item
   * @param kind
   * @param key
   * @param subKey
   */
  public static getOperators(kind: string, key: string, subKey: string): any[] {
    const category = this.findCategoryByKind(kind);
    if (!category) {
      Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
    }
    const instance = CategoryMapping[category][kind];
    if (!instance) {
      return Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
    }
    // e.g  instance['meta'].getPropInfo('tenant'), instance is ClusterCluster object
    const propInfo = instance[key].getPropInfo(subKey);
    if (!propInfo) {
      return Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
    } else {
         const dataType = propInfo.type;
         if (! dataType) {
          return Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
         }
         // For debug, use -console.log('SearchUtil.getOperators() ', kind, key, subKey, dataType);
         switch (dataType.toLowerCase()) {
          case 'string':
          case 'Array<string>': {
            return SearchUtil.stringOperators;
          }
          case 'date':
          case 'number':
          case 'Array<date>':
          case 'Array<number>':  {
            return SearchUtil.numberOperators;
          }
          case 'boolean':
          case 'Array<number>': {
            return SearchUtil.booleanOperators;
          }
          default: {
            // this blocks includes [ case 'object':]
            return Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
          }
        }
    }
  }

  /**
   * Object model looks like
   * "meta": {
   *            "name": "dev",
   *             "tenant": "default",
   *             "resource-version": "1097",
   *             "uuid": "0fc5f896-864b-4df9-9e02-8b9cd1be3ea6",
   *             "labels": {
   *                "_category": "Cluster"
   *            },
   * User can input name=dev
   * UI will make it as:
   *  "fields": {
   *   "requirements": [
   *     {
   *       "key": "meta.name",
   *       "operator": "equals",
   *      "values": [
   *         "node9"
   *       ]
   *     }
   *   ]
   * }
   * @param inputString
   * @param isField
   */
  public static padKey(inputString: string, isField: boolean): string {
    if (!inputString ) {
      return inputString;
    }
    if (isField) {
      if (inputString.startsWith(SearchUtil.SEARCHFIELD_META) || inputString.startsWith(SearchUtil.SEARCHFIELD_STATUS) || inputString.startsWith(SearchUtil.SEARCHFIELD_STATUS)) {
        return inputString;
      }
      const metaORspec = this.isMetaAttribute(inputString) ? 'meta' : 'spec';
      if (!inputString.startsWith(metaORspec)) {
        return metaORspec + '.' + inputString.trim();
      }
    } else {
      if (!inputString.startsWith('meta.labels')) {
        return inputString.trim();
      } else  {
        return 'meta.labels.' + inputString.trim();
      }
    }
    return inputString;
  }

  public static isMetaAttribute(inputString: string): boolean {
    if (SearchUtil.META_ATTRIBUTESS.indexOf(inputString.trim()) >= 0) {
      return true;
    }
    return false;
  }
}
