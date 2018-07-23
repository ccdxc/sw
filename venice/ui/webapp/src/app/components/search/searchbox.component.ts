import { Component, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { SearchComponent } from './search.component';

import { CommonComponent } from '../../common.component';
import { Utility } from '@app/common/Utility';
import { SearchUtil } from '@app/common/SearchUtil';
import { ControllerService } from '@app/services/controller.service';
import { SearchService } from '@app/services/generated/search.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';

import { SearchSearchResponse, SearchSearchRequest, ApiStatus, SearchSearchQuery_categories, SearchSearchQuery_kinds } from '@sdk/v1/models/generated/search';


/**
 * This component provides the search service UI. It wraps a search-widget and invoke search to feed data to search-widget.
 * Venice-UI search data flow is encapsulated in this component.
 * The search-widget  extends from primeNG autocomplete widget.
 *
 * Venice-UI provide guided search and free-form search.
 *   Search has simple grammar as in:xxx,yyy is:aaa,bbb,ccc has:ee,fff tag:mm,nnn
 *
 *    in : for categories
 *    is: for kinds
 *    has: for fields
 *    tag: for labels
 *
 * For example, if user enters a string "in:cluster is:node has:name=node9,tenant=default tag:os=esx,storage=ssd”,
 *
 * it means to search
 *
 *  categories  = cluster
 *  kinds = node
 *  node’s fields
 *    meta.name = node9
 *    meta.tenant = default
 *  node’s labels
 *    meta.labels.os=esx
 *    meta.labels.storage=ssd
 *
 * The request JSON for SEARCH REST API is following (copy and paste the follow JSON string to http://jsoneditoronline.org/ to expand JSON for better view)
 *
 * {"max-results":50,"query":{"categories":["Cluster"],"kinds":["Node"],"fields":{"requirements":[{"Key":"meta.name","Operator":"equals","Values":["node9"]},{"Key":"meta.tenant","Operator":"equals","Values":["default"]}]},"labels":{"requirements":[{"Key":"meta.labels.os","Operator":"equals","Values":["esx"]},{"Key":"meta.labels.storage","Operator":"equals","Values":["ssd"]}]}}}
 *
 *  Data follow:
 *  A.
 *  1. When user first clicks on search.  UI will show a list for user to pick search on [cateogry, kind, fields, labels, etc]
 *  2. User can select from [cateogry, kind, field, label], UI will provide further suggestions.
 *  B.
 *  User can type in search string, UI will exam the current search context to build suggestion accordingly.
 *  For example:
 *    input is [in:cluster is:]  UI will figure out user wants to search on "cluster" category and try to find objects of cluster. UI will load "Node, SmartNic, etc.."
 *
 *  User hits [ENTER] key, UI will build search payload and invoke SEARCH REST API.
 *
 * UI tries to be smart about keep user staying in suggestion panel or go to search-result UI.
 *
 * Say, the curretn input-string is 'in:cluster is:Node'.  We build a {'start':in, 'end', 'is'} ..
 * Base on 'in:cluster is:Node'. we make further suggestions
 *
 *
 * Key APIs
 * onSearchVeniceApplicationSelect(..) // handles when user selects item from suggestion list
 * filterVeniceApplicationSearchSuggestions(..) // generates suggestions
 * onSeachInputClick(..)  // handles when user clicks on search-input box
 *
 *
 * TODO: 2018-07-16
 * We will introduce guided search like gmail advance search UI. That feature will be in subsequent PRs.
 *
 */

interface SearchSuggestion {
  name: string;
  label?: string;
  sample?: string;
  searchType: SearchsuggestionTypes;
  count?: number;
}

enum  SearchsuggestionTypes {
  KINDS= 'kinds',
  CATEGORIES= 'categories',
  INIT = 'init',
  OP_IN= 'in',
  OP_IS = 'is',
  OP_HAS = 'has',
  OP_TAG= 'tag'
}


@Component({
  selector: 'app-searchbox',
  templateUrl: './searchbox.component.html',
  styleUrls: ['./searchbox.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SearchboxComponent extends CommonComponent implements OnInit {

  // widget properties
  searchVeniceApplication: any;
  noSearchSuggestion: String = 'no search suggestion';
  searchSuggestions: SearchSuggestion[] = [];

  @ViewChild('search') _searchwidget: SearchComponent;

  constructor(
    protected _controllerService: ControllerService,
    protected _searchService: SearchService) {
    super();
  }



  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
  }

  /**
   * This API generates suggestions
   *  It runs when search-input-string is more than two characters.
   */
  filterVeniceApplicationSearchSuggestions(event: any) {
    const searchstring = event.query;
    const formattedString = this.formatInputString(searchstring);
    if (formattedString !== searchstring) {
      this.setSearchInputString(formattedString);
    }
    if (formattedString.length > 2) {
      this.getVeniceApplicationSearchSuggestions(formattedString);
    }
  }

  /**
   * This a
   * @param value
   *
   * handle cases like:
   * in:cluster and hello, world is:wonder
   */
  protected parseInput(value: string): any {
    const inputstr = this.formatInputString(value);
    const output = [];
    if (!this.isSearchInputStartsWithGrammarTag(inputstr)) {
      return output;
    }
    const list = inputstr.split(' ');
    let prevObj = null;
    for (let i = 0; list && i < list.length; i++) {
      const listStr = list[i];
      if (listStr && listStr.trim().length > 0) {
        const strs = listStr.split(':');
        if (strs.length === 2) {
          const obj = {};
          obj['type'] = strs[0];
          obj['value'] = strs[1];
          output.push(obj);
          prevObj = obj;
        } else {
          if (prevObj) {
            prevObj['value'] = prevObj['value'] + ' ' + listStr;
          }
        }
      }
    }
    return output;
  }

  /**
   * This API reformats input string (string middle space,etc ', ' -> ',')
   * It tries to correct user input space error and make the search-input-string grammarically correct.
   *
   * @param value
   */
  private formatInputString(value: string): string {
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
   * This API build suggestion when user clicks on search input-box
   * @param event
   */
  onSeachInputClick(event: any) {
    this.buildSearchSuggestions(event);
  }

  private buildSearchSuggestions(value) {
    if (!this.searchSuggestions || this.searchSuggestions.length === 0) {
      this.buildInitSuggestions();
    } else {
      if (Utility.isEmpty(this.getSearchInputString())) {
        this.buildInitSuggestions();
      }
    }
    this.displaySuggestionPanelVisible(true);
  }


  private buildInitSuggestions(): any {
    this.searchSuggestions = this.buildInitSearchSuggestions();
  }

  /**
  * This API call server to fetch search suggestions
  * @param inputSearchString
  */
  protected getVeniceApplicationSearchSuggestions(inputSearchString: any) {
    const searchGrammarGroup = this.parseInput(inputSearchString);  // parse search-string to see if it contains search-grammar keywords (in:, is:, etc)
    let payload = null;
    if (searchGrammarGroup.length === 0) {
      payload = this.buildTextSearchPayload(inputSearchString);
      const payloadJSON = JSON.stringify(payload);
      this._callSearchRESTAPI(payloadJSON, inputSearchString);
    } else {
      const tag = this.getFirstLastSearchGrammarTag(inputSearchString);  // Say, the curretn input-string is 'in:cluster is:Node'.  We build a {'start':in, 'end', 'is'}
      this.getSearchSuggestionsByGrammarTag(tag, searchGrammarGroup);    // base on 'in:cluster is:Node'. we make suggestions
    }
  }

  /**
   *
   * @param tag
   * tag.start and tag.end
   */
  protected getSearchSuggestionsByGrammarTag(tag: any, grammarlist: any) {
    const endTagType = tag.end;
    if (endTagType) {
      // case-1: "in: cluster is:"
      switch (endTagType) {
        case 'in:':
          this.buildCategorySuggestions();
          break;
        case 'is:':
          this.buildKindSuggestions();
          break;
        default:
          return;
      }
    } else {
      // case-2: "in: cluster is:nod"
      const lastGrammarItem = grammarlist.slice(-1)[0];
      const grammarType = lastGrammarItem.type;
      const grammarValue = lastGrammarItem.value;
      if (Utility.isEmpty(grammarValue)) {
        switch (grammarType) {
          case SearchsuggestionTypes.OP_IN:
            this.buildCategorySuggestions();
            break;
          case SearchsuggestionTypes.OP_IS:
            this.buildKindSuggestions();
            break;
          default:
            return;
        }
      } else {
        switch (grammarType) {
          case SearchsuggestionTypes.OP_IN:
            this.buildCategorySuggestions();
            break;
          case SearchsuggestionTypes.OP_IS:
            this.buildKindSuggestions();
            break;
          default:
            return;
        }
      }
    }
  }

  buildSearchSearchRequest(payloadJSON): SearchSearchRequest {
    const payloadObject = JSON.parse(payloadJSON);
    const searchSearchRequest = new  SearchSearchRequest(payloadObject);

    return searchSearchRequest;
  }

  /**
   * Call REST API to fetch data.
   * Depending it is in suggestion-mode or search-result mode, we deal with response accordingly.
   */
  private _callSearchRESTAPI(payloadJSON: string, searched: any, suggestionMode: boolean = true) {
    this._searchService.QueryPost(this.buildSearchSearchRequest(payloadJSON)).subscribe(response => {
      const status = response.statusCode;
      const body: SearchSearchResponse = response.body as SearchSearchResponse;
      if (status === 200) {
        if (suggestionMode) {
          this._processGlobalSearchResult(searched, body);  // response.body
          this.displaySuggestionPanelVisible(true);
        } else {
          const payload = {
            id: 'searchresult',
            result: response.body,
            searched: searched,
            searchrequest: payloadJSON
          };
          this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA] = payload;
          this._controllerService.publish(Eventtypes.SEARCH_RESULT_LOAD_REQUEST, payload);
          this.displaySuggestionPanelVisible(false);
        }
      } else {
        // TODO: suppose to use APIStatus - const apiStatus: ApiStatus = response.body as ApiStatus;
        console.error(this.getClassName() + '_callSearchRESTAPI()' + response);
      }
    }, err => {
      this.successMessage = '';
      this.errorMessage = 'Failed to retrieve data! ' + err;
      this.error(err);
    });
  }


  /**
   * Build payload for text Search REST-API.
   * @param searched
   */
  protected buildTextSearchPayload(searched: string) {
    return {
      'max-results': 50,
      'query': {
        'texts': [
          {
            'text': [
              searched
            ]
          }
        ]
      }
    };
  }

  /**
   * This function builds request json for invoking Search API
   * It should examine the current search context to decide the type of search. (by category, kind, label, fields,etc)
   * @param searched
   */
  protected buildComplexSearchPayload(list: any[], searched: string): any {
    let payload = {
      'max-results': 50,
      'query': {
      }
    };
    for (let i = 0; i < list.length; i++) {
      const obj = list[i];
      switch (obj.type) {
        case SearchsuggestionTypes.OP_IN:
          payload.query['categories'] = this.buildCategoriesPayload(obj);
          break;
        case SearchsuggestionTypes.OP_IS :
          payload.query['kinds'] = this.buildKindsPayload(obj);
          break;
        case SearchsuggestionTypes.OP_HAS:
          payload.query['fields'] = {
            'requirements': this.buildFieldsPayload(obj)
          };
          break;
        case SearchsuggestionTypes.OP_TAG:
          payload.query['labels'] = {
            'requirements': this.buildLabelsPayload(obj)
          };
          break;
        default:
          console.log(this.getClassName() + 'buildComplexSearchPayload() does not recognize ' + searched);
          payload = this.buildTextSearchPayload(searched);
      }
    }
    return payload;
  }

  private buildCategoriesPayload(obj: any): any {
    const output = [];
    const values = obj.value.split(',');
    for (let i = 0; i < values.length; i++) {
      const exprStr = values[i];
      output.push(Utility.makeFirstLetterUppercase(exprStr));
    }
    return output;
  }
  private buildKindsPayload(obj): any {
    return this.buildCategoriesPayload(obj);
  }
  private buildFieldsPayload(obj): any {
    return this.buildFieldsLabelsPayloadHelper(obj, true);
  }

  private buildFieldsLabelsPayloadHelper(obj: any, isField: boolean): any {
    const output = [];
    const values = obj.value.split(',');
    for (let i = 0; i < values.length; i++) {
      const exprStr = values[i];
      const expr = this.buildExpression(exprStr, isField);
      if (expr) {
        output.push(expr);
      }
    }
    return output;
  }

  private buildLabelsPayload(obj): any {
    return this.buildFieldsLabelsPayloadHelper(obj, false);
  }

  /**
   * Build expression for searching "field" and "label".
   */
  buildExpression(inputString: string, isField: boolean): any {
    const operators = (isField) ? SearchUtil.SEARCH_FIELD_OPERATORS : SearchUtil.SEARCH_LABEL_OPERATORS;
    let obj = null;
    for (let i = 0; i < operators.length; i++) {
      const op = operators[i];
      if (inputString.indexOf(op.operator) >= 0) {
        const strs = inputString.split(op.operator);
        obj = {};
        // cases sensitive.
        obj['key'] = this.padKey(strs[0], isField);
        obj['operator'] = op.label;
        obj['values'] = [strs[1].trim()];
      }
    }
    return obj;
  }

  /**
   * Check whether current input search string starts with prammar tag. like "is:xxx"
   */
  protected isSearchInputStartsWithGrammarTag(inputString: string): boolean {
    return this.isSearchInputStartsEndsWithGrammarTag(inputString, true);
  }

  /**
   * Check whether current input search string end with prammar tag. like "is:xxx has:"
   */
  protected isSearchInputEndsWithGrammarTag(inputString: string): boolean {
    return this.isSearchInputStartsEndsWithGrammarTag(inputString, false);
  }

  /**
   * Helper function
   * @param inputString
   * @param isStartsWith
   */
  protected isSearchInputStartsEndsWithGrammarTag(inputString: string, isStartsWith: boolean): boolean {
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
  protected getFirstLastSearchGrammarTag(inputString: string) {
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
  private padKey(inputString: string, isField: boolean): string {
    if (isField) {
      if (!inputString.startsWith('meta.')) {
        return 'meta.' + inputString.trim();
      }
    } else {
      if (!inputString.startsWith('meta.')) {
        return 'meta.labels.' + inputString.trim();
      }
    }
    return inputString;
  }


  /**
   * This functions processes server provided search suggestions
   * @param inputSearchString
   * @param data
   */
  protected _processGlobalSearchResult(inputSearchString: string, data: any) {
    this._processGlobalSearchResultHelper(inputSearchString, data);
  }

  /**
   *
   * @param inputSearchString
   * @param data
   *
   *  TODO : need more work to process search REST-API response
   */
  protected _processGlobalSearchResultHelper(inputSearchString: string, data: any): any {
    const entries = data['entries'];
    const list = [];
    const map = {};
    for (let i = 0; entries && i < entries.length; i++) {
      entries[i].name = entries[i].meta.name;
      list.push(entries[i]);
      const kind = entries[i]['kind'];
      if (map[kind]) {
        map[kind] = map[kind] + 1;
      } else {
        map[kind] = 1;
      }
    }
    let distinct = Utility.getUniqueValueFromArray(list, 'kind');
    distinct = distinct.sort();
    const outputList = [];
    for (let j = 0; j < distinct.length; j++) {
      outputList.push(
        {
          name: distinct[j],
          count: map[distinct[j]]
        }
      );
    }
    this.searchSuggestions = outputList;
    return outputList;  // list;
  }

  /**
   * This API build up request JSON and call Search REST API
   * @param event
   */
  onInvokeSearch(event) {
    const searched = event.text;
    const mode = event.mode;
    if (Utility.isEmpty(this.getSearchInputString())) {
      return; // do nothing
    }
    const output = this.parseInput(searched);  // parse search-string to see if it contains search-grammar keywords (in:, is:, etc)
    let payload = null;
    if (output.length === 0 || !this.isSearchInputStartsWithGrammarTag(searched)) {
      payload = this.buildTextSearchPayload(searched);
    } else {
      payload = this.buildComplexSearchPayload(output, searched);
    }
    const payloadJSON = JSON.stringify(payload);
    this._callSearchRESTAPI(payloadJSON, searched, mode);
  }

  /**
   * This api handle what to do when user select item from suggestion panel
   * @param selection
   */
  onSearchVeniceApplicationSelect(selection: any) {
    if (!selection ) {
      return ;
    }
    const searchType = selection.searchType;
    switch (searchType) {
      case SearchsuggestionTypes.INIT:
        this.handleSelect_Init(selection);
        break;
      case SearchsuggestionTypes.OP_IN:
        this.handleSelectCategory(selection);
        break;
      case SearchsuggestionTypes.OP_IS:
        this.handleSelectKind(selection);
        break;
      case SearchsuggestionTypes.OP_HAS:
        this.handleSelectField(selection);
        break;
      case SearchsuggestionTypes.OP_TAG:
        this.handleSelectLabel(selection);
        break;
      default:
        this.handleSelectDefaults(selection);
        break;
    }
  }

  protected handleSelect_Init(selection) {
    if (!selection || !selection.searchType)  {
      return;
    }
    this.searchVeniceApplication.length = 0;
    const type = (selection.name) ;
    switch (type) {
      case 'category':
        this.setSearchInputString(this.getSearchInputString() + ' ' + 'in:');
        this.buildCategorySuggestions();
        this.displaySuggestionPanelVisible(true);
        this._searchwidget.show();
        break;
      case 'kind':
        this.setSearchInputString(this.getSearchInputString() + ' ' + 'is:');
        this.buildKindSuggestions();
        this.displaySuggestionPanelVisible(true);
        break;
      case 'field':
        this.setSearchInputString(this.getSearchInputString() + ' ' + 'has:');
        this.searchSuggestions.length = 0;
        break;
      case 'label':
        this.setSearchInputString(this.getSearchInputString() + ' ' + 'tag:');
        this.searchSuggestions.length = 0;
        break;
      default:
        return {};
    }
  }

  protected updateSearchInputString(grammarTag: string, selection: any) {
    const searched = this.getSearchInputString();
    const selectedOption = selection.name;
    if (Utility.isEmpty(searched)) {
      this.setSearchInputString(grammarTag + ':' + selectedOption);
      return;
    }
    const grammarlist = this.parseInput(searched);
    if (grammarlist.length === 0) {
      this.setSearchInputString(this.getSearchInputString() + ' ' + grammarTag + '' + selectedOption);
    }
    const tag = this.getFirstLastSearchGrammarTag(searched);
    const endTagType = tag['end'];
    if (endTagType) {
      // case-1: "in: cluster is:"
      this.setSearchInputString(this.getSearchInputString() + selectedOption);
    } else {
      // case-2: "in: cluster is:nod"
      const lastGrammarItem = grammarlist.slice(-1)[0];
      const grammarType = lastGrammarItem.type;
      const grammarValue = lastGrammarItem.value;
      if (Utility.isEmpty(grammarValue)) {
        this.setSearchInputString(this.getSearchInputString() + selectedOption);
      } else {
        // TODO: check if grammarValue is valid
        this.setSearchInputString(this.getSearchInputString() + ',' + selectedOption);
      }
    }
  }

  protected handleSelectCategory(selection: any) {
    const selectedOption = selection.name; // (selection.name) ? selection.name : selection.selectedOption;
    this.updateSearchInputString(SearchsuggestionTypes.OP_IN, selection);
  }

  protected handleSelectKind(selection) {
    const selectedOption = selection.name; // (selection.name) ? selection.name : selection.selectedOption;
    this.updateSearchInputString(SearchsuggestionTypes.OP_IS, selection);
  }

  protected handleSelectField(selection) {
    this.updateSearchInputString(SearchsuggestionTypes.OP_HAS, selection);
  }

  protected handleSelectLabel(selection) {
    this.updateSearchInputString(SearchsuggestionTypes.OP_TAG, selection);
  }


  /**
   * selection is "{"name":"AuthenticationPolicy","count":1}"
   * @param selection
   */
  protected handleSelectDefaults(selection: any) {
    if (selection.name) {
      this.updateSearchInputString(SearchsuggestionTypes.OP_IS, selection);
      return this.searchByKind(selection);
    } else {
      return;
    }
  }

  /**
   * Search by KIND
   * @param selection : SearchSuggestion
   */
  protected searchByKind(selection: SearchSuggestion) {
    this.searchByHelper(selection, SearchsuggestionTypes.KINDS);
  }

  protected searchByCategory(selection: SearchSuggestion) {
    this.searchByHelper(selection, SearchsuggestionTypes.CATEGORIES);
  }

  /**
   * With input  this.searchByHelper(selection, 'Kinds'), This API make a JSON as
   * {"max-results":50,"query":{"Kinds":["SecurityGroup"]}} // see search API spec
   *
   * searchBy is 'KIND'
   */
  protected searchByHelper(selection: SearchSuggestion, searchBy: string) {
    const searchname = selection.name;
    const payload = this.buildSearchPayloadWithSuggestion(searchname, selection, searchBy);
    const payloadJSON = JSON.stringify(payload);
    this._callSearchRESTAPI(payloadJSON, searchname);
  }

  protected buildSearchPayloadWithSuggestion(search: any, selection: SearchSuggestion, querytype: string): any {
    const obj = {
      'max-results': 50,
      'query': {}
    };
    obj.query[querytype] = [
      this.buildQueryContent(search)
    ];
    return obj;
  }

  protected buildQueryContent(search: string): string {
    if (search === 'security-groups') {
      return 'SecurityGroups';
    }
    return Utility.makeFirstLetterUppercase(search);  // TODO: REST should support case insensitivity
  }

  displaySuggestionPanelVisible(toDispaly: boolean) {
    this._searchwidget.panelVisible = toDispaly;
  }

  displayItem(item): string {
    return JSON.stringify(item, null, 2);
  }

  displaySelectedItem(item): string {
    return (item) ? item.name : '';
  }

  displaySelectedItemTooltip(item) {
    return this.displayItem(item);
  }

  protected buildInitSearchSuggestions(): any {
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

  protected buildCategorySuggestions(): any {
    const keys = this.getCategories();
    this.searchSuggestions = this.buildCategory_kindSuggestionHelper(keys, 'in');
  }

  private buildCategory_kindSuggestionHelper(keys: string[], searchType) {
    const list = [];
    for (let i = 0; i < keys.length; i++) {
      const obj = {
        name: keys[i],
        searchType: searchType
      };
      list.push(obj);
    }
    return list;
  }

  buildKindSuggestions(): any {
    const keys = this.getKinds();
    this.searchSuggestions = this.buildCategory_kindSuggestionHelper(keys, 'is');
  }

  private getCategories(): string[] {
    return Object.keys(SearchSearchQuery_categories);
  }
  private getKinds(): string[] {
    return Object.keys(SearchSearchQuery_kinds);
  }

  protected setSearchInputString(text: string) {
    this._searchwidget.setInputText(text);
  }

  protected getSearchInputString(): string {
    return this._searchwidget.getInputText();
  }
}
