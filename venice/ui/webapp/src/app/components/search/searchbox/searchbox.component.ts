import { Component, OnInit, ViewChild, ViewEncapsulation, OnDestroy } from '@angular/core';
import { SearchComponent } from '@app/components/search/search/search.component';

import { CommonComponent } from '@app/common.component';
import { Utility } from '@app/common/Utility';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { ControllerService } from '@app/services/controller.service';
import { SearchService } from '@app/services/generated/search.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { SearchsuggestionTypes, SearchSuggestion, SearchSpec, SearchExpression, GuidedSearchCriteria } from '@app/components/search';
import { SearchSearchResponse, SearchSearchRequest } from '@sdk/v1/models/generated/search';

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
 *
 */


@Component({
  selector: 'app-searchbox',
  templateUrl: './searchbox.component.html',
  styleUrls: ['./searchbox.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SearchboxComponent extends CommonComponent implements OnInit, OnDestroy {

  // widget properties
  searchVeniceApplication: any;
  noSearchSuggestion: String = 'no search suggestion';
  searchSuggestions: SearchSuggestion[] = [];
  subscriptions = {};
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
    // set up subscriptions
    this.subscriptions[Eventtypes.SEARCH_SET_SEARCHSTRING_REQUEST] = this._controllerService.subscribe(Eventtypes.SEARCH_SET_SEARCHSTRING_REQUEST, (payload) => {
      this.onSetSearchStringRequest(payload);
    });

    this.subscriptions[Eventtypes.SEARCH_OPEN_GUIDEDSERCH_REQUEST] = this._controllerService.subscribe(Eventtypes.SEARCH_OPEN_GUIDEDSERCH_REQUEST, (payload) => {
      this.onOpenGuidedSearchRequest(payload);
    });
  }

  ngOnDestroy(): void {
    Object.keys(this.subscriptions).forEach((item) => {
      if (this.subscriptions[item]) {
        this.subscriptions[item].unsubscribe();
      }
    });
  }

  /**
   * This API responses to subcrition
   * @param payload
   */
  onSetSearchStringRequest(payload: any) {
    if (this._searchwidget && payload && payload.text) {
      this._searchwidget.setInputText(payload.text);
    }
  }

  /**
   * This API responses to subcrition
   * @param payload
   */
  onOpenGuidedSearchRequest(payload: any) {
    if (this._searchwidget && payload) {
      this._searchwidget.showGuidedSearch(null);
    }
  }

  /**
   * This API generates suggestions
   *  It runs when search-input-string is more than two characters.
   */
  filterVeniceApplicationSearchSuggestions(event: any) {
    const searchstring = event.query;
    let formattedString = SearchUtil.formatInputString(searchstring);
    // We don't want to change user search input while user is typing. But we want to buidl suggestion using valid criteria in order to avoid search REST API error.
    if (formattedString.length > 2) {
      formattedString = this.cleanupInputstring(formattedString);
      this.getVeniceApplicationSearchSuggestions(formattedString);
    }
    this._searchwidget.loading = false;
  }

  /**
   * This a
   * @param value
   *
   * handle cases like:
   * in:cluster and hello, world is:wonder
   */
  protected parseInput(value: string): any {
    const inputstr = SearchUtil.formatInputString(value);
    let output = [];
    if (!SearchUtil.isSearchInputStartsWithGrammarTag(inputstr)) {
      return output;
    }
    output = SearchUtil.parseSearchStringToObjectList(inputstr);
    return output;
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
      if (Utility.isEmpty(this.getSearchInputString())) {
        this.buildInitSuggestions();
      }
    } else {
      if (Utility.isEmpty(this.getSearchInputString())) {
        this.buildInitSuggestions();
      } else {
        this.getVeniceApplicationSearchSuggestions(this.getSearchInputString());
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
      const tag = SearchUtil.getFirstLastSearchGrammarTag(inputSearchString);  // Say, the curretn input-string is 'in:cluster is:Node'.  We build a {'start':in, 'end', 'is'}
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
    const searchSearchRequest = new SearchSearchRequest(payloadObject);

    return searchSearchRequest;
  }

  /**
   * Call REST API to fetch data.
   * Depending it is in suggestion-mode or search-result mode, we deal with response accordingly.
   */
  private _callSearchRESTAPI(payloadJSON: string, searched: any, suggestionMode: boolean = true) {
    this._searchService.PostQuery(this.buildSearchSearchRequest(payloadJSON)).subscribe(response => {
      const status = response.statusCode;
      const body: SearchSearchResponse = response.body as SearchSearchResponse;
      this._searchwidget.loading = false;
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
          this._searchwidget.isInGuidedSearchMode = false; // close guided-search overlay panel
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
      this._searchwidget.loading = false;
      if (!this._searchwidget.isInGuidedSearchMode) {
        this.searchSuggestions.length = 0;
        this.displaySuggestionPanelVisible(true);
      } else {
        this._searchwidget.isInGuidedSearchMode = false; // close guided-search overlay panel
        const payload = { id: 'searchresult' }; // empty payload
        this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA] = payload;
        this._controllerService.publish(Eventtypes.SEARCH_RESULT_LOAD_REQUEST, payload);
        this.displaySuggestionPanelVisible(false);
      }
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
        case SearchsuggestionTypes.OP_IS:
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
      let exprStrCategory = values[i];
      exprStrCategory = Utility.makeFirstLetterUppercase(exprStrCategory);
      if (SearchUtil.isValidCategory(exprStrCategory)) {
        output.push(exprStrCategory);
      }
    }
    return output;
  }

  private buildKindsPayload(obj): any {
    const output = [];
    const values = obj.value.split(',');
    for (let i = 0; i < values.length; i++) {
      let exprStrKind = values[i];
      exprStrKind = Utility.makeFirstLetterUppercase(exprStrKind);
      if (SearchUtil.isValidKind(exprStrKind)) {
        output.push(exprStrKind);
      }
    }
    return output;
  }

  private buildFieldsPayload(obj): any {
    return this.buildFieldsLabelsPayloadHelper(obj, true);
  }

  private buildLabelsPayload(obj): any {
    return this.buildFieldsLabelsPayloadHelper(obj, false);
  }

  buildFieldsLabelsPayloadHelper(obj: any, isField: boolean): any {
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

  /**
   * Build expression for searching "field" and "label".
   */
  buildExpression(inputString: string, isField: boolean): SearchExpression {
    const searchExpression = SearchUtil.parseToExpression(inputString, isField);
    searchExpression.key = SearchUtil.padKey(searchExpression.key, isField);
    return searchExpression;
  }


  /**
   * This functions processes server provided search suggestions
   * @param inputSearchString
   * @param data
   */
  protected _processGlobalSearchResult(inputSearchString: string, data: SearchSearchResponse) {
    this._processGlobalSearchResultHelper(inputSearchString, data);
  }

  /**
   *
   * @param inputSearchString
   * @param data
   *
   *  Return an array which looks like
   *  [
   *   {"Node":1},{"Event":20}
   *  ]
   */
  protected _processGlobalSearchResultHelper(inputSearchString: string, data: SearchSearchResponse): any {
    // Due to PR5959 change, we have to process data.entries to get to the JSON object level
    const objects = data.entries;
    const entries = [];
    if (!objects) {
      this.searchSuggestions.length = 0; // search retrun no data. So we clear out the suggestions.
      return entries;
    }
    for (let k = 0; k < objects.length; k++) {
      entries.push(objects[k].object); // objects[k] is a SearchEntry object
    }

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
    return outputList;
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
    const grammarList = this.parseInput(searched);  // parse search-string to see if it contains search-grammar keywords (in:, is:, etc)
    let payload = null;
    // if (grammarList.length === 0 || !this.isSearchInputStartsWithGrammarTag(searched)) {
    if (this.shouldRunTextSearch(searched)) {
      payload = this.buildTextSearchPayload(searched);
    } else {
      const cleangrammarList = this.cleanupSearchInput(grammarList);
      payload = this.buildComplexSearchPayload(cleangrammarList, searched);
      const newSearchInput = this.cleangrammarListToString(cleangrammarList);
      if (!Utility.isEmpty(newSearchInput)) {
       this.setSearchInputString(newSearchInput);
      } else {
        // input can be "in:abc" - where abc is not a valid category
        payload = this.buildTextSearchPayload(searched);
      }
    }
    const payloadJSON = JSON.stringify(payload);
    this._callSearchRESTAPI(payloadJSON, searched, mode);
  }

  private cleangrammarListToString(cleangrammarList: any, isAllowEmpty: boolean = false) {
    const strlist = [];
    cleangrammarList.filter(obj => {
      if (!Utility.isEmpty(obj.value)) {
        strlist.push(obj.type + ':' + obj.value);
      } else {
        if (isAllowEmpty) {
          strlist.push(obj.type + ':' + obj.value);
        }
      }
    });
    const newSearchInput = strlist.join(' ');
    return newSearchInput;
  }

  /**
   * Clean up input-string
   * @param inputString
   */
  private cleanupInputstring(inputString: string): string {
    let outputString = inputString;
    if (Utility.isEmpty(inputString)) {
      return outputString; // do nothing
    }
    const grammarList = this.parseInput(inputString);
    if (this.shouldRunTextSearch(inputString)) {
      return outputString;
    } else {
      const cleangrammarList = this.cleanupSearchInput(grammarList);
      const cleanString = this.cleangrammarListToString(cleangrammarList, true);
      if (! Utility.isEmpty(cleanString)) {
        outputString = cleanString;  // prevent input such as "is:,ahello " to mess up.
      }
    }
    // This is for debug - // console.log (this.getClassName() + '.cleanupInputstring()', inputString, outputString);
    return outputString;
  }

  /**
   * This API clean up input string to ensure searched kind and category are valid.
   * For example
   * Input as in:Cluster,abc is:Node,world has:meta.name=node1,namespace=default tag:_category=Cluster  (abc, world are not valid)
   * to
   * in:Cluster is:Node has:meta.name=node1,namespace=default tag:_category=Cluster
   */
  private cleanupSearchInput(inputlist: any[]): any {
    const list = Utility.getLodash().cloneDeep(inputlist);
    for (let i = 0; i < list.length; i++) {
      const obj = list[i];
      switch (obj.type) {
        case SearchsuggestionTypes.OP_IN:
          obj['value'] = this.buildCategoriesPayload(obj).join(',');
          break;
        case SearchsuggestionTypes.OP_IS:
          obj['value'] = this.buildKindsPayload(obj).join(',');
          break;
        default:
          break;
      }
    }
    return list;
  }

  /**
   * Decside whether to run text search or guided search
   * @param inputSearchString
   */
  private shouldRunTextSearch(inputSearchString: string): boolean {
    const grammarList = this.parseInput(inputSearchString);
    if (grammarList.length === 0 || !SearchUtil.isSearchInputStartsWithGrammarTag(inputSearchString)) {
      return true;
    } else {
      if (grammarList.length === 1) {
        // in input is like "in:", it is a text search
        const value = grammarList[0]['value'];
        if (Utility.isEmpty(value)) {
          return true;
        }
      } else {
        return false;
      }
    }
    return false;
  }



  /**
   * This api handle what to do when user select item from suggestion panel
   * @param selection
   */
  onSearchVeniceApplicationSelect(selection: any) {
    if (!selection) {
      return;
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
    if (!selection || !selection.searchType) {
      return;
    }
    this.searchVeniceApplication.length = 0;
    const type = (selection.name);
    const preFix = SearchUtil.getSearchInitPrefix(selection);
    this.setSearchInputString(this.getSearchInputString() + ' ' + preFix);
    switch (type) {
      case 'category':
        // TODO: figure out how to display overlay
        this.buildCategorySuggestions();
        this.displaySuggestionPanelVisible(true);
        break;
      case 'kind':
        this.buildKindSuggestions();
        this.displaySuggestionPanelVisible(true);
        break;
      case 'field':
        this.searchSuggestions.length = 0;
        break;
      case 'label':
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
      this.setSearchInputString(this.getSearchInputString() + ' ' + grammarTag + ':' + selectedOption);
    }
    const tag = SearchUtil.getFirstLastSearchGrammarTag(searched);
    const endTagType = tag['end'];
    if (endTagType) {
      // case-1: "in: cluster is:"
      this.setSearchInputString(this.getSearchInputString() + selectedOption);
    } else {
      // case-2: "in: cluster is:nod"
      const lastGrammarItem = grammarlist.slice(-1)[0];
      const grammarType = (lastGrammarItem) ? lastGrammarItem.type : null;
      const grammarValue = (lastGrammarItem) ? lastGrammarItem.value : null;
      if (this.getSearchInputString().endsWith(selectedOption)) {
        // for example, existing text is "is:Node", selection is "Node".  There is  no need to update.
        return;
      }
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
      // Invoke get search result
      this.setSearchInputString(SearchsuggestionTypes.OP_IS + ':' + selection.name);
      this.searchByKind(selection, false);
      this.displaySuggestionPanelVisible(false);
    } else {
      return;
    }
  }

  /**
   * Search by KIND
   * @param selection : SearchSuggestion
   */
  protected searchByKind(selection: SearchSuggestion, suggestionMode: boolean = true) {
    this.searchByHelper(selection, SearchsuggestionTypes.KINDS);
  }

  protected searchByCategory(selection: SearchSuggestion, suggestionMode: boolean = true) {
    this.searchByHelper(selection, SearchsuggestionTypes.CATEGORIES);
  }

  /**
   * With input  this.searchByHelper(selection, 'Kinds'), This API make a JSON as
   * {"max-results":50,"query":{"Kinds":["SecurityGroup"]}} // see search API spec
   *
   * searchBy is 'KIND'
   */
  protected searchByHelper(selection: SearchSuggestion, searchBy: string, suggestionMode: boolean = true) {
    const searchname = selection.name;
    const payload = this.buildSearchPayloadWithSuggestion(searchname, selection, searchBy);
    const payloadJSON = JSON.stringify(payload);
    this._callSearchRESTAPI(payloadJSON, searchname, suggestionMode);
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
    this._searchwidget.overlayVisible = toDispaly;
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
    return SearchUtil.getCategories();
  }
  private getKinds(): string[] {
    return SearchUtil.getKinds();
  }

  protected setSearchInputString(text: string) {
    this._searchwidget.setInputText(text);
  }

  protected getSearchInputString(): string {
    return this._searchwidget.getInputText();
  }

  /**
   * This API serves HTML template
   *
   * It handles search request from guided-search panel
   * @param guidedSearchCriteria
   *
   * guidedSearchCriteria is a JSON object. It looks like
   * {"in":["Network","Auth"],"is":["Certificate","FwlogPolicy"],"has":[{"keyFormControl":"name","operatorFormControl":"equal","valueFormControl":"hello"}],"tag":[{"keyFormControl":"text","operatorFormControl":"equal","valueFormControl":"esx","keytextFormName":"os"},{"keyFormControl":"text","operatorFormControl":"equal","valueFormControl":"ssd","keytextFormName":"storage"}]}
   * We convert it to a search-input string as:
   * in:Network,Auth is:Certificate,FwlogPolicy has:name=hello tag:os=esx,storage=ssd
   * Then we run invoke Search REST API to get search-result
   */
  onInvokeGuidedSearch(guidedSearchCriteria: GuidedSearchCriteria) {
    const searchInputString = this.getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria);
    if (Utility.isEmpty(searchInputString)) {
      return;
    }
    const grammarList = this.parseInput(searchInputString);
    const payload = this.buildComplexSearchPayload(grammarList, searchInputString);
    const payloadJSON = JSON.stringify(payload);
    this.setSearchInputString(searchInputString);
    this._callSearchRESTAPI(payloadJSON, searchInputString, false);
  }

  private getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria: GuidedSearchCriteria): string {
    const inStr = (guidedSearchCriteria[SearchsuggestionTypes.OP_IN] && guidedSearchCriteria[SearchsuggestionTypes.OP_IN].length > 0) ? SearchsuggestionTypes.OP_IN + ':' + guidedSearchCriteria[SearchsuggestionTypes.OP_IN].join(',') : '';
    const isStr = (guidedSearchCriteria[SearchsuggestionTypes.OP_IS] && guidedSearchCriteria[SearchsuggestionTypes.OP_IS].length > 0) ? SearchsuggestionTypes.OP_IS + ':' + guidedSearchCriteria[SearchsuggestionTypes.OP_IS].join(',') : '';
    const hasStr = this.getHasStringFromGuidedSearchSpec(guidedSearchCriteria);
    const tagStr = this.getTagStringFromGuidedSearchSpec(guidedSearchCriteria);
    const list = [inStr, isStr, hasStr, tagStr];
    const searchInputString = list.join(' ').trim();
    return searchInputString;
  }

  /**
   * extract out the 'has' configs
   */
  private getHasStringFromGuidedSearchSpec(guidedsearchCriteria: any): any {
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
  private getTagStringFromGuidedSearchSpec(guidedsearchCriteria: any): any {
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
}
