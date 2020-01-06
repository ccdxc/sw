import {SearchExpression, SearchInputTypeValue, SearchModelField, SearchSpec} from '@app/components/search';
import {Utility} from '@app/common/Utility';
import {CategoryMapping} from '@sdk/v1/models/generated/category-mapping.model';
import {
  CompileSearchInputStringResult,
  ExamineCategoryOrKindResult,
  GuidedSearchCriteria,
  SearchGrammarItem,
  SearchInputErrors,
  SearchSuggestion,
  SearchsuggestionTypes
} from './';
import {TableCol} from '@components/shared/tableviewedit';
import {RepeaterData, RepeaterItem, ValueType} from 'web-app-framework';
import {AdvancedSearchExpression} from '@components/shared/advanced-search';
import {AdvancedSearchComponent} from '@components/shared/advanced-search/advanced-search.component';

export interface LocalFields {
  field: string;
  singleSelect: boolean;
}

interface SearchCols {
  repeaterData: RepeaterData[];
  localFields: {[key: string]: LocalFields};
}
export class SearchUtil {
  public static LAST_SEARCH_DATA = 'last_search_data';
  public static SEARCH_MAX_RESULT: number = 500;

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

  public static SPECIAL_EVENT_KINDS = ['Event', 'AuditEvent'];

  // This is a helper map to find the keyword for advancedSearchParser
  // and taking action on detected keyword with it's own processFunc
  public static advancedSearchKeywordMap: {
    field: {
      processFunc: (input: string) => AdvancedSearchExpression;
    }
  } = {
    field: {
      processFunc: (input: string): AdvancedSearchExpression => {
        const endIdx = input.indexOf(';');
        const first = input.substr(0, endIdx).trim(), last = input.substr(endIdx + 1).trim();
        const exp = SearchUtil.parseToExpressionAdvancedSearch(first.split(':')[1]);
        return {searchExpressions: [exp], generalSearch: [], remainingString: last};
      }
    }
  };

  // For the backend, equal expects only one value, while in can support multiple
  // The UI hides this distinction and shows equal, but allows user to select multiple if they wish to.
  public static stringOperators = [
    { label: 'equals', value: 'in' },
    { label: 'not equals', value: 'notIn' },
  ];

  public static numberOperators = [
    { label: '=', value: 'in' },
    { label: '!=', value: 'notIn' },
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
   *
   *  Cases (type in - invoke search.  type in - open guided search)
   *   is:AuditEvent has:action=login
   *   is:AuditEvent has:action==login
   *   is:AuditEvent has:action!=login
   *   is:AuditEvent has:action=~login
   *   is:AuditEvent has:action!~login
   *
   *   guided search criteria can be reformat to search string expression (see convertSearchSpecOperator(..) API)
   */
  public static SEARCH_FIELD_OPERATORS = [
    { 'operator': '>=', 'label': 'gte', 'description': 'greater than or equal', 'searchoperator': 'gte' },
    { 'operator': '<=', 'label': 'lte', 'description': 'less than or equal', 'searchoperator': 'lte' },
    { 'operator': '=', 'label': 'equals', 'description': 'equal', 'searchoperator': 'in' },
    { 'operator': '==', 'label': 'equals', 'description': 'equal', 'searchoperator': 'in' },
    { 'operator': '!=', 'label': 'not equals', 'description': 'not equal', 'searchoperator': 'notIn' },
    { 'operator': '>', 'label': 'gt', 'description': 'greater than', 'searchoperator': 'gt' },
    { 'operator': '<', 'label': 'lt', 'description': 'less than', 'searchoperator': 'lt' },
    { 'operator': '=~', 'label': 'in', 'description': 'contains', 'searchoperator': 'in' },
    { 'operator': '!~', 'label': 'not in', 'description': 'not in', 'searchoperator': 'notIn' }
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

  public static isKindInSpecialEventList(kind: string): boolean {
    return this.SPECIAL_EVENT_KINDS.includes(kind);
  }

  public static isKindListContainSpecialEvent(kinds: string[]): boolean {
    for (let i = 0; i < kinds.length; i++) {
      if (this.isKindInSpecialEventList(kinds[i])) {
        return true;
      }
    }
    return false;
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
    const instance = CategoryMapping[category][kind].instance;
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
      if (opStr === op.description || opStr === op.label || opStr === op.searchoperator) {
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
   *  //inputstr.replace(':', '\:');   // 0242.c0a8.1c02 to 02\:42\:c0\:a8\:1c\:02
   * @param value
   */
  public static searchEncodeText(value: string): string {
    if (value == null) {
      return value;
    }

    let inputstr = value.trim();
    if (inputstr.indexOf(':') >= 0 || inputstr.indexOf('/') >= 0) {
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
            // case like  is:DistributedServiceCard has:meta.name=~4444.4444.0002
            searchGrammarItem.type = strs[0];
            const idx = listStr.indexOf(':'); // has: <- ":"
            const strs2 = listStr.substr(idx + 1); // get meta.name=~4444.4444.0002
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
        break;  // operators has "<=" ahead of "<".  So if inputString is a<=b, we want [a, <=, b] instead of [a, <, =b]
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
    const instance = CategoryMapping[category][kind].instance;
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
        inputString.startsWith(SearchUtil.SEARCHFIELD_SPEC) ||
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

  public static isGuidedSearchCriteriaEmpty(obj: GuidedSearchCriteria): boolean {
    return (!obj.in || obj.in.length === 0) && (!obj.is || obj.is.length === 0) && (!obj.tag || obj.tag.length === 0) && (!obj.has || obj.has.length === 0);
  }

  /**
   * extract out the 'has' configs
   */
  public static getHasStringFromGuidedSearchSpec(guidedsearchCriteria: any): any {
    const list = [];
    const type = SearchsuggestionTypes.OP_HAS;
    const hasSpecList = guidedsearchCriteria[type];
    if (!hasSpecList) {
      return '';
    }
    hasSpecList.filter((repeaterValueItem) => {
      if (!Utility.isEmpty(repeaterValueItem.keyFormControl) && !Utility.isEmpty(repeaterValueItem.operatorFormControl) && !Utility.isEmpty(repeaterValueItem.valueFormControl)) {
        const str = repeaterValueItem.keyFormControl + SearchUtil.convertSearchSpecOperator(repeaterValueItem.operatorFormControl) + repeaterValueItem.valueFormControl;
        list.push(str);
      }
    });
    return (list.length > 0) ? type + ':' + list.join(',') : '';
  }

  /**
   * extract out the 'tag' configs
   */
  public static getTagStringFromGuidedSearchSpec(guidedsearchCriteria: any): any {
    const list = [];
    const type = SearchsuggestionTypes.OP_TAG;
    const tagSpecList = guidedsearchCriteria[type];
    if (!tagSpecList) {
      return '';
    }
    tagSpecList.filter((repeaterValueItem) => {
      if (!Utility.isEmpty(repeaterValueItem.keytextFormName) && !Utility.isEmpty(repeaterValueItem.operatorFormControl) && !Utility.isEmpty(repeaterValueItem.valueFormControl)) {
        const str = repeaterValueItem.keytextFormName + SearchUtil.convertSearchSpecOperator(repeaterValueItem.operatorFormControl) + repeaterValueItem.valueFormControl;
        list.push(str);
      }
    });
    return (list.length > 0) ? type + ':' + list.join(',') : '';
  }

  public static getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria: GuidedSearchCriteria): string {
    const inStr = (guidedSearchCriteria[SearchsuggestionTypes.OP_IN] && guidedSearchCriteria[SearchsuggestionTypes.OP_IN].length > 0) ? SearchsuggestionTypes.OP_IN + ':' + guidedSearchCriteria[SearchsuggestionTypes.OP_IN].join(',') : '';
    const isStr = (guidedSearchCriteria[SearchsuggestionTypes.OP_IS] && guidedSearchCriteria[SearchsuggestionTypes.OP_IS].length > 0) ? SearchsuggestionTypes.OP_IS + ':' + guidedSearchCriteria[SearchsuggestionTypes.OP_IS].join(',') : '';
    const hasStr = SearchUtil.getHasStringFromGuidedSearchSpec(guidedSearchCriteria);
    const tagStr = SearchUtil.getTagStringFromGuidedSearchSpec(guidedSearchCriteria);
    const list = [inStr, isStr, hasStr, tagStr];
    const searchInputString = list.join(' ').trim();
    return searchInputString;
  }

  /**
   * This a utility function.
   * @param value
   *
   * It handles cases like:
   * in:cluster and hello, world is:wonder
   */
  public static parseSearchInputString(value: string): any {
    const inputstr = SearchUtil.formatInputString(value);
    return SearchUtil.compileSearchInputString(inputstr).list;
  }

  public static buildSearchFieldsLabelsPayloadHelper(obj: any, isField: boolean, isEvent = false): any {
    const output = [];
    const values = obj.value.split(',');
    let prevExp: SearchExpression = null;
    // support case like "has:name=~Liz,test,tenant=default"
    for (let i = 0; i < values.length; i++) {
      const exprStr = values[i];
      const expr = SearchUtil.parseToExpression(exprStr, isField, isEvent);
      if (expr) {
        output.push(expr);
        prevExp = expr;
      } else {
        if (prevExp) {
          prevExp.values.push(exprStr);
        }
      }
    }
    return output;
  }

  // Specifying a kind in the tableCol data takes precedence, but if its null it defaults to the kind passed into the function.
  public static tableColsToRepeaterData(cols: TableCol[], kind?: string): SearchCols {
    const searchData: SearchCols = {
      repeaterData: [],
      localFields: {}
    };
    cols.forEach(ele => {
      // if user dont allow search, we dont build it in the RepeaterData aray
      if (ele.disableSearch) {
        return;
      }
      if (ele.localSearch) {
        searchData.localFields[ele.header] = {
          field: ele.field,
          singleSelect: false
        };
      }

      let kindTemp: string;
      if (ele.kind) {
        kindTemp = ele.kind;
      } else if (kind) {
        kindTemp = kind;
      } else {
        // if both kind in col and kind from param is not given, skip
        return;
      }

      // dynamic detect op
      const op = ele.advancedSearchOperator ? ele.advancedSearchOperator : SearchUtil.getOperators(kindTemp, ele.field.split('.'));

      searchData.repeaterData.push({
        key: {label: ele.header, value: ele.header},
        operators: op,
        valueType: ValueType.inputField
      });
    });
    return searchData;
  }

  public static  parseToExpressionAdvancedSearch(inputString: string): SearchExpression {
    const operators = SearchUtil.SEARCH_FIELD_OPERATORS;
    const searchExpression: SearchExpression = { key: null, operator: null, values: null };
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (inputString.indexOf(op.operator) >= 0) {
        const strs = inputString.split(op.operator);
        searchExpression.key = strs[0];
        searchExpression.operator = op.searchoperator;
        searchExpression.values = [strs[1].trim()];
      }
    }
    return (searchExpression.key) ? searchExpression : null;
  }


  // This is for custom syntax compiling in advanced search component
  // TODO: support case detection for all lower case
  public static advancedSearchCompiler(fieldInputs: any[], generalSearch: string): string {
    const list = [];
    fieldInputs.forEach((repeaterValueItem) => {
      if (!Utility.isEmpty(repeaterValueItem.keyFormControl) && !Utility.isEmpty(repeaterValueItem.operatorFormControl) && !Utility.isEmpty(repeaterValueItem.valueFormControl)) {
        const str = repeaterValueItem.keyFormControl + SearchUtil.convertSearchSpecOperator(repeaterValueItem.operatorFormControl) + repeaterValueItem.valueFormControl;
        list.push(`field:${str};`);
      }
    });
    // token and value
    const fields = (list.length > 0) ? list.join(' ') : '';
    const texts = (generalSearch) ? generalSearch : '';

    return `${texts}${(fields && texts) ? ' ' : ''}${fields}`;
  }
  // This is for custom syntax parsing in advanced search component
  // Parsing util to convert a string to AdvancedSearchExpression
  // example:
  // input = "test hello field:Who=~dasdsa; field:Act On (kind)=~1,2;"
  // the parsed output will be
  // {
  //   searchExpressions: [{"key":"Who","operator":"in","values":["dasdsa"]},{"key":"Act On (kind)","operator":"in","values":["1,2"]}];
  //   generalSearch: ['test', 'hello'];
  // }
  // TODO: support case detection for all lower case
  public static advancedSearchParser(inputStr: string): AdvancedSearchExpression {
    if (!inputStr) {
      return null;
    }
    const result = {searchExpressions: [], generalSearch: []} as AdvancedSearchExpression;
    let foundKeyword = false;

    // default processFunc if no match
    let processFunc = (input: string): AdvancedSearchExpression => {
      let endIdx = input.indexOf(' ');
      if (endIdx === -1) {
        endIdx = input.length;  // not found
      }
      const first = input.substr(0, endIdx).trim(), last = input.substr(endIdx + 1).trim();
      return {searchExpressions: [], generalSearch: [first], remainingString: last};
    };

    // overwriting processFunc if we found a keyword
    Object.keys(SearchUtil.advancedSearchKeywordMap).forEach(key => {
      if (inputStr.indexOf(key) === 0) {
        foundKeyword = true;
        processFunc = SearchUtil.advancedSearchKeywordMap[key].processFunc;
      }
    });

    // process current chunk
    let appendResult: AdvancedSearchExpression = processFunc(inputStr);
    result.searchExpressions = result.searchExpressions.concat(appendResult.searchExpressions);
    result.generalSearch = result.generalSearch.concat(appendResult.generalSearch);

    // throw remaining into the recursive
    appendResult = this.advancedSearchParser(appendResult.remainingString);
    if (appendResult) {
      result.searchExpressions = result.searchExpressions.concat(appendResult.searchExpressions);
      result.generalSearch = result.generalSearch.concat(appendResult.generalSearch);
    }

    return result;
  }

  /**
   * Build place-holder text for repeater-item
   *
   * @param repeater
   * @param keyFormName
   */
  public static buildFieldValuePlaceholder(repeater: RepeaterItem, keyFormName: string) {
    // TODO: may change this once we have enhanced category-mapping.ts
    const key = repeater.formGroup.value[keyFormName];
    if (key.startsWith(SearchUtil.SEARCHFIELD_META)) {
      if (key.indexOf('time') > -1) {
        return 'YYYY-MM-DDTHH:mm:ss.sssZ';
      }
    }
    if (key.startsWith(SearchUtil.SEARCHFIELD_SPEC)) {
      if (key.indexOf('-ip') > -1) {
        return 'xxx.xxx.xxx.xxx';
      }
    }
    if (key.startsWith(SearchUtil.SEARCHFIELD_STATUS)) {
      if (key.indexOf('time') > -1) {
        return 'YYYY-MM-DDTHH:mm:ss.sssZ';
      }
      if (key.indexOf('date') > -1) {
        return 'YYYY-MM-DD';
      }
    }
    return key;
  }

  public static buildCategoriesPayload(obj: any): any {
    return SearchUtil.filterOutInvalidCategoryOrKind(obj, true);
  }

  public static buildKindsPayload(obj: SearchGrammarItem): any {
    return SearchUtil.filterOutInvalidCategoryOrKind(obj, false);
  }

  public static buildFieldsPayload(obj, isEvent = false): any {
    return SearchUtil.buildSearchFieldsLabelsPayloadHelper(obj, true, isEvent);
  }

  public static buildLabelsPayload(obj): any {
    return SearchUtil.buildSearchFieldsLabelsPayloadHelper(obj, false);
  }

  /**
   * Splits search text
   * @param str string to split
   */
  public static splitString(str: string): string[] {
    const arr = [], newArr = [];
    str.split('"').forEach( (substr, i) => {
      if (substr !== '') {
        if (i % 2 === 0) {
          arr.push(...substr.split(' ').filter(x => x !== ''));
        } else {
          arr.push(substr);
        }
      }
    });
    arr.forEach( text => {
      newArr.push('"' + text + '"');
    });
    return newArr;
  }

    /**
   * Build payload for text Search REST-API.
   * @param searched
   */
  public static buildTextSearchPayload(searched: string) {
    // encode special characters like MAC string 0242.c0a8.1c02
    const texts = (typeof searched === 'string') ? [SearchUtil.searchEncodeText(searched)] : SearchUtil.searchEncodeText(searched);
    // const texts = (typeof searched === 'string') ? SearchUtil.splitString(searched) : SearchUtil.searchEncodeText(searched);
    // TODO: Need to add a fix for separating string by [", comma, space] in a sequence
    /*
    for query search:
    'abc, "abc, pqr" efg uvw' =>
    'query': {
      'texts': [
        {
          'text': ["abc"]
        },
        {
          'text': [""abc, pqr"","efg","uvw"]
        }
      ]
    }
    */
    return {
      'max-results': SearchUtil.SEARCH_MAX_RESULT,
      'query': {
        'texts': [
          {
            'text': texts
          }
        ]
      }
    };
  }

    /**
   * Generate text search criteria
   * @param obj
   * encode special characters like MAC string 0242.c0a8.1c02
   */
  public static generateSearchTexts(obj: SearchGrammarItem): any {
    // obj.value is "node1,default"
    if (typeof obj.value === 'string') {
      const str = obj.value  as string;
      const texts = str.split(',');
      const newList = [];
      texts.filter((str1) => {
        const myObj = {};
        myObj['text'] = [SearchUtil.searchEncodeText(str1)];
        newList.push(myObj);
      });
      return newList;
    }
    // obj.value is "[node1,defaul pen]"
    if (Array.isArray(obj.value)) {
      const newList = [];
      const strList = [];
      obj.value.filter ( (str1) => {
        strList.length = 0;
        const texts = str1.split(',');
        texts.filter((str2) => {
          strList.push(SearchUtil.searchEncodeText(str2));
        });
        const myObj = {};
        myObj['text'] = Utility.getLodash().cloneDeep(strList);
        newList.push(myObj);
      });
      return newList;
    }
    return [];
  }

    /**
   * This function builds request json for invoking Search API
   * It should examine the current search context to decide the type of search. (by category, kind, label, fields,etc)
   * @param searched
   */
  public static buildComplexSearchPayload(list: any[], searched: string): any {
    let payload = {
      'max-results': SearchUtil.SEARCH_MAX_RESULT,
      'query': {
      }
    };
    // We evaluate the has operations last so that we
    // know if the object kind is an event or not.
    const fieldRequirementIndexes = [];
    for (let i = 0; i < list.length; i++) {
      const obj: SearchGrammarItem = list[i];
      switch (obj.type) {
        case SearchsuggestionTypes.OP_IN:
          payload.query['categories'] = this.buildCategoriesPayload(obj);
          break;
        case SearchsuggestionTypes.OP_IS:
          payload.query['kinds'] = this.buildKindsPayload(obj);
          break;
        case SearchsuggestionTypes.OP_HAS:
          fieldRequirementIndexes.push(i);

          break;
        case SearchsuggestionTypes.OP_TAG:
          payload.query['labels'] = {
            'requirements': this.buildLabelsPayload(obj)
          };
          break;
        case SearchsuggestionTypes.OP_TXT:
          const texts = this.generateSearchTexts(obj);
          payload.query['texts'] = texts;
          break;
        default:
          console.log('buildComplexSearchPayload() does not recognize ' + searched);
          payload = SearchUtil.buildTextSearchPayload(searched);
      }
    }
    fieldRequirementIndexes.forEach((index) => {
      const obj = list[index];
      const isEvent = payload.query['kinds'] != null && payload.query['kinds'].length === 1 && SearchUtil.isKindInSpecialEventList(payload.query['kinds'][0]);
        // checking if (payload.query['kinds'][0] === 'Event' || payload.query['kinds'][0] === 'AuditEvent') ;
      payload.query['fields'] = {
        'requirements': this.buildFieldsPayload(obj, isEvent)
      };
    });
    return payload;
  }

  // Example:
  // orderValue = {
  // requirements: [{
  //   keyFormControl: "key",
  //   keytextFormName "env"   -- when we have text-input label selector, keyFormControl is always "text", keytextFormName contains the user input
  //   operatorFormControl: "in",
  //   valueFormControl: ["1"]}]
  // }
  public static convertFormArrayToSearchExpression(value: any, addMetatag: boolean = false): SearchExpression[] {
    const data = value;
    if (data == null) {
      return null;
    }

    let retData = data.filter((item) => {
      return (!Utility.isEmpty(item.keyFormControl) || !Utility.isEmpty(item.keytextFormName) ) && !Utility.isEmpty(item.valueFormControl) && item.valueFormControl.length !== 0;
    });
    // make sure the value field is an array
    retData = retData.map((item) => {
      const tag = !!(item.keytextFormName) ? item.keytextFormName : item.keyFormControl;
      const keyValue = ((addMetatag) ? 'meta.labels.' : '')  + tag;
      // modify here to trim each string of the array
      const valValues = Array.isArray(item.valueFormControl) ?
        item.valueFormControl : item.valueFormControl.trim().split(',');
      const trimmedValues = valValues.map(each => (each) ? each.trim() : each);
      const searchExpression: SearchExpression = {
        key:  keyValue,
        operator: item.operatorFormControl,
        values: trimmedValues
      };
      return searchExpression;
    });
    return retData;
  }

}
