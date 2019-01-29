import { SearchInputTypeValue, SearchSpec, SearchExpression, SearchModelField } from '@app/components/search';
import { Utility } from '@app/common/Utility';
import { CategoryMapping } from '@sdk/v1/models/generated/category-mapping.model';
import { SearchSuggestion, CompileSearchInputStringResult, SearchGrammarItem, SearchsuggestionTypes, SearchInputErrors, ExamineCategoryOrKindResult , GuidedSearchCriteria} from './';

export class SearchUtil {
  public static LAST_SEARCH_DATA = 'last_search_data';

  public static EVENT_KEY_ENTER = 13;
  public static EVENT_KEY_LEFT = 37;
  public static EVENT_KEY_RIGHT = 39;
  public static EVENT_KEY_UP = 38;
  public static EVENT_KEY_DOWN = 40;
  public static EVENT_KEY_TAB = 9;
  public static EVENT_KEY_ESCAPE = 27;

  public static SEARCHFIELD_HIDDEN = ['kind', 'api-version'];
  public static SEARCHFIELD_META = 'meta';
  public static SEARCHFIELD_SPEC = 'spec';
  public static SEARCHFIELD_STATUS = 'status';

  public static META_ATTRIBUTES = ['name', 'tenant', 'generation-id', 'resource-version', 'uuid', 'creation-time', 'mod-time', 'namespace', 'self-link'];

  // For the backend, equal expects only one value, while in can support multiple
  // The UI hides this distinction and shows equal, but allows user to select multiple if they wish to.
  public static stringOperators = [
    { label: 'equals', value: 'in' },
    { label: 'not equals', value: 'not in' },
  ];

  public static numberOperators = [
    { label: '=', value: 'equals' },
    { label: '!=', value: 'not equals' },
    { label: '>', value: 'gt' },
    { label: '>=', value: 'gte' },
    { label: '<', value: 'lt' },
    { label: '<=', value: 'lte' }
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
    { 'operator': '>=', 'label': 'gte', 'description': 'greater than or equal', 'searchoperator': 'gte' },
    { 'operator': '<=', 'label': 'lte', 'description': 'less than or equal', 'searchoperator': 'lte' },
    { 'operator': '=', 'label': 'equals', 'description': 'equal', 'searchoperator': 'equals' },
    { 'operator': '==', 'label': 'equals', 'description': 'equal', 'searchoperator': 'equals' },
    { 'operator': '!=', 'label': 'not equals', 'description': 'not equal', 'searchoperator': 'not equals' },
    { 'operator': '>', 'label': 'gt', 'description': 'greater than', 'searchoperator': 'gt' },
    { 'operator': '<', 'label': 'lt', 'description': 'less than', 'searchoperator': 'lt' },
    { 'operator': '=~', 'label': 'in', 'description': 'contains', 'searchoperator': 'in' },
    { 'operator': '!~', 'label': 'not in', 'description': 'not contains', 'searchoperator': 'not in' }
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

  public static isValidKind(kind: string): boolean {
    return (Utility.getKinds().indexOf(kind) > -1);
  }

  public static isValidCategory(kind: string): boolean {
    return (Utility.getCategories().indexOf(kind) > -1);
  }

  /**
   * Find Venice-SDK object model informaton
   * @param category
   * @param kind
   */
  public static getModelInfo(category: string, kind: string): SearchModelField {
    if (!category) {
      return null;
    }
    const instance = CategoryMapping[category][kind];
    if (!instance) {
      return null;
    }

    const obj = instance;
    // Removing keys that shouldn't be seen by user
    this.SEARCHFIELD_HIDDEN.forEach(field => {
      if (field in obj) {
        delete obj[field];
      }
    });
    return obj;
  }

  public static getModelInfoByKind(kind: string): any {
    const cat = Utility.findCategoryByKind(kind);
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
    const operators = SearchUtil.SEARCH_FIELD_OPERATORS;
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (opStr === op.description || opStr === op.label) {
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
    const operators = SearchUtil.SEARCH_FIELD_OPERATORS;
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (opStr === op.operator) {
        return op.label;
      }
    }
    return '';
  }

  /**
   * ':' is reserved character is search. We want to encode the search to make back-end-search work in MAC string
   *  //inputstr.replace(':', '\:');   // 02:42:c0:a8:1c:02 to 02\:42\:c0\:a8\:1c\:02
   * @param value
   */
  public static searchEncodeText(value: string): string {
    if (value == null) {
      return value;
    }
    let inputstr = value.trim();
    if (inputstr.indexOf(':') >= 0) {
      inputstr = encodeURIComponent(inputstr);
    }
    return inputstr;
  }

  /**
     * This API reformats input string (string middle space,etc ', ' -> ',')
     * It tries to correct user input space error and make the search-input-string grammarically correct.
     *
     * @param value
     */
  public static formatInputString(value: string): string {
    if (value == null) {
      return value;
    }
    let inputstr = value.trim(); // "in:cluster is: node  has:meta.name=node9 ";
    inputstr = inputstr.replace(/\s\s+/g, ' ');
    inputstr = inputstr.replace(';', ':');   // format [is;node]  double [is:node]
    inputstr = inputstr.replace(',,', ',');  // format [in: cluster is,,monitoring is:node]  double ",,"
    inputstr = inputstr.replace(', ', ',');  // format [in:cluster,  network is:node,tenants]
    inputstr = inputstr.replace(' ,', ',');  // format [in:cluster ,network is:node,tenants]
    inputstr = inputstr.replace(': ', ':');  // format [is:node has: name=node9   in:cluster]
    inputstr = inputstr.replace(' :', ':');  // format [in:cluster is :node,monitoring]
    inputstr = inputstr.replace('= ', '=');  // format [is:node has: name= node9   in:cluster]
    inputstr = inputstr.replace(' =', '=');  // format [is:node has: name =node9   in:cluster]
    inputstr = inputstr.replace('is:in:', 'is: in:');
    inputstr = inputstr.replace('in:is:', 'in: is:');
    inputstr = inputstr.replace('is:is:', 'is:');
    inputstr = inputstr.replace('in:in:', 'in:');
    inputstr = inputstr.replace(/\s\s+/g, ' ');
    return inputstr;
  }

  /**
   * inputstr as:
   *  in:Cluster is:Node has:name=node9 tag:os=esx,storage=ssd
   * output is: CompileSearchInputStringResult
   *  CompileSearchInputStringResult.list is
   *  "[{"type":"in","value":"Cluster"},{"type":"is","value":"Node"},{"type":"has","value":"name=node9"},{"type":"tag","value":"os=esx,storage=ssd"}]"
   * CompileSearchInputStringResult.searchspec is a SearchSpec interface
   * @param inputstr
   */

  public static compileSearchInputString(inputstring: string): CompileSearchInputStringResult {
    const inputstr = SearchUtil.formatInputString(inputstring);
    const list = inputstr.split(' ');
    const outputArray = [];
    const texts = [];
    const searchSpec: SearchSpec = {};
    const compileSearchInputStringResult: CompileSearchInputStringResult = {};
    for (let i = 0; list && i < list.length; i++) {
      const listStr = list[i];
      if (listStr && listStr.trim().length > 0) {
        const strs = listStr.split(':');
        const searchGrammarItem: SearchGrammarItem = {};
        if (strs.length === 2) {
          searchGrammarItem.type = strs[0];
          searchGrammarItem.value = strs[1];
          outputArray.push(searchGrammarItem);
          searchSpec[strs[0]] = (searchSpec[strs[0]] === undefined) ? strs[1] : searchSpec[strs[0]] + ',' + strs[1];
        } else {
          if (this.hasOperatorInString(listStr)) {
              // case like  is:SmartNIC has:meta.name=~44:44:44:44:00:02
              searchGrammarItem.type = strs[0];
              const idx = listStr.indexOf(':'); // has: <- ":"
              const strs2 = listStr.substr(idx + 1); // get meta.name=~44:44:44:44:00:02
              outputArray.push(searchGrammarItem);
              searchSpec[strs[0]] = (searchSpec[strs[0]] === undefined) ? strs2 : searchSpec[strs[0]] + ',' + strs2;
          } else {
            texts.push(listStr);
          }
        }
      }
    }
    if (texts.length > 0) {
      const searchGrammarItem: SearchGrammarItem = {};
      searchGrammarItem.type = 'txt';
      searchGrammarItem.value = texts;
      outputArray.push(searchGrammarItem);
      searchSpec.txt = searchGrammarItem.value;
    }
    // If input is "node is:Event in:Cluster,Node is:Node"
    // searchSpec is {"is":"Event,Node","in":"Cluster,Node","txt":["node"]}
    // outputArray is [{"type":"is","value":"Event"},{"type":"in","value":"Cluster,Node"},{"type":"is","value":"Node"},{"type":"txt","value":["node"]}]
    // we want make:
    // [{"type":"is","value":"Event,Node"},{"type":"in","value":"Cluster,Node"},{"type":"txt","value":["node"]}]
    const newOutputArray = [];
    const ref = {};
    outputArray.filter((searchGrammarItem: SearchGrammarItem) => {
      if (!ref[searchGrammarItem.type]) {
        ref[searchGrammarItem.type] = true;
        searchGrammarItem.value = searchSpec[searchGrammarItem.type];
        newOutputArray.push(searchGrammarItem);
      }
    });

    compileSearchInputStringResult.list = newOutputArray;
    compileSearchInputStringResult.searchspec = searchSpec;
    compileSearchInputStringResult.input = inputstring;
    compileSearchInputStringResult.freeformtext = texts;
    compileSearchInputStringResult.error = SearchUtil.catchInvalidCategoryOrKind(searchSpec);
    return compileSearchInputStringResult;
  }

  public static hasOperatorInString(inputstring: string): boolean {
    const operators = SearchUtil.SEARCH_FIELD_OPERATORS;
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (inputstring.indexOf(op.operator) >= 0) {
        return true;
      }
    }
    return false;
  }

  /**
   * Catch any invalid category/kind inputs
   * @param searchSpec
   */
  public static catchInvalidCategoryOrKind(searchSpec: SearchSpec): SearchInputErrors {
    const error: SearchInputErrors = {};
    const inValue = searchSpec.in;
    const isValue = searchSpec.is;

    const catValues = (inValue) ? inValue.split(',') : null;
    const kindValues = (isValue) ? isValue.split(',') : null;

    if (catValues) {
      error.in = this.examineCategoryOrKind(catValues, true).error.in;
    }
    if (kindValues) {
      error.is = this.examineCategoryOrKind(kindValues, false).error.is;
    }
    error.messages = [];
    if (error.in) {
      error.messages.push('Invaid Input Categories: ' + error.in);
    }
    if (error.is) {
      error.messages.push('Invaid Input Kinds: ' + error.is);
    }
    return (Utility.isEmptyObject(error)) ? null : error;
  }

  /**
   * Examine user inputed category or kind,
   * take out any invalid category/kind inputs
   * report any input errors.
   */
  public static examineCategoryOrKind(values: any[], isCategory: boolean): any {
    const error: SearchInputErrors = {};
    const output = [];
    for (let i = 0; values && i < values.length; i++) {
      let exprStr = values[i];
      exprStr = Utility.makeFirstLetterUppercase(exprStr);
      if (isCategory) {
        if (SearchUtil.isValidCategory(exprStr)) {
          output.push(exprStr);
        } else {
          error.in = (error.in === undefined) ? exprStr : error.in + ',' + values[i];
        }
      } else {
        if (SearchUtil.isValidKind(exprStr)) {
          output.push(exprStr);
        } else {
          error.is = (error.is === undefined) ? exprStr : error.is + ',' + values[i];
        }
      }
    }
    const outObj: ExamineCategoryOrKindResult = {
      output: output,
      error: error,
      type: (isCategory) ? 'in' : 'is'
    };
    return outObj;
  }

  /**
   * Take out any invalid category/kind inputs
   */
  public static filterOutInvalidCategoryOrKind(obj: SearchGrammarItem, isCategory: boolean): any {
    const objValue = (typeof obj.value === 'string') ? obj.value : obj.value.join(',');
    const values = objValue.split(',');
    return this.examineCategoryOrKind(values, isCategory).output;
  }
  /**
   * When user picks a suggestion, we want to update the search-input box text.
   *
   * Say existing input is "in:cluster is:Node", user picks "is:"
   */
  public static buildSearchInputStringFromSuggestion(searched: string, grammarTag: string, selection: any): string {
    const selectedOption = selection.name;
    let outputString = searched;
    if (Utility.isEmpty(searched)) {
      return grammarTag + ':' + selectedOption;
    }
    // If input is "has:name=~Liz,test is:User", client picks "User" kind, we don't want to change.
    if (!this.isSearchInputAlreadyContain(searched, grammarTag, selection)) {
      searched = searched + ' ' + grammarTag + ':' + selectedOption;
    }
    const formattedString = SearchUtil.formatInputString(searched);
    const compiled: CompileSearchInputStringResult = SearchUtil.compileSearchInputString(formattedString);
    outputString = SearchUtil.searchGrammarListToString(compiled.list);
    return outputString;
  }

  /**
   * If input is "has:name=~Liz,test is:User", client picks "User" kind, this API will return true.
   * @param searchInputString
   * @param grammarTag
   * @param selection
   */
  public static isSearchInputAlreadyContain(searchInputString: string, grammarTag: string, selection: any): boolean {
    const compiled: CompileSearchInputStringResult = SearchUtil.compileSearchInputString(searchInputString);
    if (!compiled.searchspec) {
      return false;
    }
    const grammarValues = compiled.searchspec[grammarTag];
    if (grammarValues) {
      const selectedOption = (selection.name) ? selection.name : selection;
      if (grammarValues.indexOf(selectedOption) >= 0) {
        return true;
      }
    }
    return false;
  }

  /**
   * input is like:
   *  "[{"type":"in","value":"Cluster"},{"type":"is","value":"Node"},{"type":"has","value":"name=node9"},{"type":"tag","value":"os=esx,storage=ssd"}]"
   *
   * output as:
   *  in:Cluster is:Node has:name=node9 tag:os=esx,storage=ssd
   * output is: CompileSearchInputStringResult
   */
  public static searchGrammarListToString(cleangrammarList: any, isAllowEmpty: boolean = false): string {
    const strlist = [];
    cleangrammarList.forEach(obj => {
      if (obj.type === SearchsuggestionTypes.OP_TXT) {
        if (!Utility.isEmpty(obj.value)) {
          strlist.push(obj.value);
        }
      } else {
        if (!Utility.isEmpty(obj.value)) {
          strlist.push(obj.type + ':' + obj.value);
        } else {
          if (isAllowEmpty) {
            strlist.push(obj.type + ':' + obj.value);
          }
        }
      }
    });
    const newSearchInput = [];
    strlist.forEach((item) => {
      if (Array.isArray(item)) {
        let strArray = '';
        item.filter((str) => {
          strArray = strArray + ' ' + str;
        });
        newSearchInput.push(strArray.trim());
      } else {
        newSearchInput.push(item);
      }
    });
    return newSearchInput.join(' ');
  }

  /**
   * When user just enters the search-input box, we call this API to build initial suggestion
   */
  public static buildInitSearchSuggestions(): any {
    const list = [
      {
        name: 'category',
        label: '(in: Monitoring, Cluster, Security, Network, etc)',
        sample: 'in:cluster',
        searchType: SearchsuggestionTypes.INIT
      },
      {
        name: 'kind',
        label: '(is: Events, Alert, Security-Group, Node, Tenant, etc)',
        sample: 'is:node',
        searchType: SearchsuggestionTypes.INIT
      },
      {
        name: 'field',
        label: '(has: name, tenant, etc)',
        sample: 'has:name=node9',
        searchType: SearchsuggestionTypes.INIT
      },
      {
        name: 'label',
        label: '(tag: os, storage, etc)',
        sample: 'tag:os=esx',
        searchType: SearchsuggestionTypes.INIT
      }
    ];
    return list;
  }



  /**
   * convert SearchInputObjectList ToSearchSpec
   * list is like
   *  "[{"type":"in","value":"Cluster"},{"type":"is","value":"Node"},{"type":"has","value":"name=node1"},{"type":"tag","value":"os=esx,storage=ssd"}]"
   */
  public static convertToSearchSpec(list: SearchInputTypeValue[]): SearchSpec {
    const searchSpec: SearchSpec = {};
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
  public static isSearchInputStartsWithGrammarTag(inputString: string): boolean {
    return this.isSearchInputStartsEndsWithGrammarTag(inputString, true);
  }

  /**
   * Check whether current input search string end with prammar tag. like "is:xxx has:"
   */
  public static isSearchInputEndsWithGrammarTag(inputString: string): boolean {
    return this.isSearchInputStartsEndsWithGrammarTag(inputString, false);
  }

  /**
   * Helper function
   * @param inputString
   * @param isStartsWith
   */
  public static isSearchInputStartsEndsWithGrammarTag(inputString: string, isStartsWith: boolean): boolean {
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
  public static getFirstLastSearchGrammarTag(inputString: string) {
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
   *
   * For events, we don't want to append spec or meta as many of the properties are inlined.
   */
  public static parseToExpression(inputString: string, isField: boolean, isEvent = false): SearchExpression {
    const operators = SearchUtil.SEARCH_FIELD_OPERATORS;
    const searchExpression: SearchExpression = { key: null, operator: null, values: null };
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (inputString.indexOf(op.operator) >= 0) {
        const strs = inputString.split(op.operator);
        // cases sensitive.
        searchExpression.key = this.padKey(strs[0], isField, isEvent);
        searchExpression.operator = op.searchoperator;
        searchExpression.values = [strs[1].trim()];
      }
    }
    return (searchExpression.key) ? searchExpression : null;
  }

  /**
   * Get operators for guided-search repeater-item
   * @param kind
   * @param key
   * @param subKey
   */
  public static getOperators(kind: string, keys: string[]): any[] {
    const category = Utility.findCategoryByKind(kind);
    const _ = Utility.getLodash();
    if (!category) {
      _.union(SearchUtil.stringOperators, SearchUtil.numberOperators);
    }
    const instance = CategoryMapping[category][kind];
    if (!instance) {
      return _.union(SearchUtil.stringOperators, SearchUtil.numberOperators);
    }
    const propInfo = Utility.getNestedPropInfo(instance, keys);
    if (!propInfo) {
      return Utility.getLodash().union(SearchUtil.stringOperators, SearchUtil.numberOperators);
    } else {
      const dataType = propInfo.type;
      if (!dataType) {
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
        case 'Array<number>': {
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
   *
   * For events, we don't want to append spec or meta as many of the properties are inlined.
   *
   * @param inputString
   * @param isField
   */
  public static padKey(inputString: string, isField: boolean, isEvent = false): string {
    if (!inputString) {
      return inputString;
    }
    if (isField) {
      if (inputString.startsWith(SearchUtil.SEARCHFIELD_META) ||
        inputString.startsWith(SearchUtil.SEARCHFIELD_STATUS) ||
        inputString.startsWith(SearchUtil.SEARCHFIELD_STATUS) ||
        isEvent) {
        return inputString;
      }
      const metaORspec = this.isMetaAttribute(inputString) ? 'meta' : 'spec';
      if (!inputString.startsWith(metaORspec)) {
        return metaORspec + '.' + inputString.trim();
      }
    } else {
      if (inputString.startsWith('meta.labels')) {
        return inputString.trim();
      } else {
        return 'meta.labels.' + inputString.trim();
      }
    }
    return inputString;
  }

  public static isMetaAttribute(inputString: string): boolean {
    if (SearchUtil.META_ATTRIBUTES.indexOf(inputString.trim()) >= 0) {
      return true;
    }
    return false;
  }

  public static isGuidedSearchCriteriaEmpty( obj: GuidedSearchCriteria): boolean {
     return (!obj.in || obj.in.length === 0) && (!obj.is || obj.is.length === 0)  && (!obj.tag || obj.tag.length === 0) && (!obj.has || obj.has.length === 0 );
  }
}
