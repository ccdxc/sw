import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { SearchUtil } from '@components/search/SearchUtil';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { SearchResultPayload } from '@app/components/search';

import { SearchSearchResponse } from '@sdk/v1/models/generated/search';
import { EventsEvent } from '@sdk/v1/models/generated/events';
import { MessageService } from 'primeng/primeng';
import { ToolbarData } from '@app/models/frontend/shared/toolbar.interface' ;
import { Router } from '@angular/router';

/**
 * SearchResult is a component hosted in VenicUI app main page.
 * When component is initialized, it expects this._controllerService.LoginUserInfo[Utility.LAST_SEARCH_DATA] has search result.
 *
 * When user invokes get search result while playing search-widget, an 'Eventtypes.SEARCH_RESULT_LOAD_REQUEST' will be fired.
 * Search-result component listens to this event and re-render search result UI.
 *
 * In case, SearchResult already in present, if user invokes search again, this search-result component will act on new search-result data.
 */

@Component({
  selector: 'app-searchresult',
  templateUrl: './searchresult.component.html',
  styleUrls: ['./searchresult.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SearchresultComponent extends BaseComponent implements OnInit, OnDestroy {
  public static LAYOUT_ROW: string = 'Row';
  public static LAYOUT_GRID: string = 'Grid';
  searchResult: any;
  categories: any[];

  searchSearchResponse: SearchSearchResponse;
  selectedCategory: any;
  selectedKind: any;

  selectedDropdown: any;

  // default layout
  layoutGrid: boolean = false;

  _started = false;

  constructor(protected _controllerService: ControllerService,
    protected messageService: MessageService,
    protected router: Router
  ) {
    super(_controllerService, messageService);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * Component life cycle API.
   * It subscribe to event.
   */
  ngOnInit() {
    if (!this._started) {
      const lastSeachData = this._controllerService.isUserLogin() ? this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA] : null;
      if (lastSeachData) {
        this.getSearchResult();
        this._started = true;
      }
    }
    this.subscriptions[Eventtypes.SEARCH_RESULT_LOAD_REQUEST] = this._controllerService.subscribe(Eventtypes.SEARCH_RESULT_LOAD_REQUEST, (payload) => {
      this.getSearchResult();
      // re-compute toolbar data if new search-result is available
      this._controllerService.setToolbarData(this.buildToolbarData());
    });
    // compute toolbar data
    this._controllerService.setToolbarData(this.buildToolbarData());
  }

  private buildToolbarData(): ToolbarData {
    const toolbarData: ToolbarData =    {
      breadcrumb: [{ label: 'Search Results', url: '' }]
    };
    // add a dropdown in toolbar if there are search-result data.
    if (this.categories && this.categories.length >= 0) {
      toolbarData.dropdowns = [
        {
          callback: (event, sbutton) => {
            this.onLayoutDropDownChange(sbutton);
          },
          options: [
            { label: SearchresultComponent.LAYOUT_ROW, value: SearchresultComponent.LAYOUT_ROW },
            { label: SearchresultComponent.LAYOUT_GRID, value: SearchresultComponent.LAYOUT_GRID }
          ],
          model: this.selectedDropdown,
          placeholder: 'Select layout'
        }
      ];
    }
    return toolbarData;
  }

  onLayoutDropDownChange(sbutton: any) {
    // w
    this.layoutGrid = (sbutton.model === SearchresultComponent.LAYOUT_GRID);
  }

  /**
   * This API assumes the last search-result is stored in this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA]
   * It process the search-result and let UI render data.
   * Structure-wise
   *   tenant -> category -> kind -> entries[]
   */
  getSearchResult() {
    this.categories = [];

    if (this._controllerService.LoginUserInfo) {
      const data: SearchResultPayload = this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA];
      if (!data) {
        return;
      }
      const result = data.result;
      const searchSearchResponse = new SearchSearchResponse(result);
      this.searchSearchResponse = searchSearchResponse;
      const tenants = searchSearchResponse['aggregated-entries'].tenants;
      if (!tenants) {
        // it means search service returns no record.
        return;
      }
      const tenantKeys = Object.keys(tenants);
      for (let i = 0; i < tenantKeys.length; i++) {
        const cat = tenants[tenantKeys[i]].categories;
        const catKeys = Object.keys(cat);
        for (let j = 0; j < catKeys.length; j++) {
          const catUIObj = {
            name: catKeys[j],
            value: cat[catKeys[j]],
            tenant: tenantKeys[i]
          };
          this.categories.push(catUIObj);
        }
      }
      // refresh the selected category and selected kind
      this.selectedCategory = this.categories[0];
      const kinds = this.getKinds(this.selectedCategory);
      if (kinds && kinds.length > 0) {
        this.selectedKind = kinds[0];
      }
    }
  }

  ngOnDestroy(): void {
    Object.keys(this.subscriptions).forEach((item) => {
      if (this.subscriptions[item]) {
        this.subscriptions[item].unsubscribe();
      }
    });
  }

  /**
   * This API serves html template. It invokes a SEARCH_SET_SEARCHSTRING_REQUEST message.  Global search box will response to it.
   * @param $event
   * @param text
   */
  searchWithGrammarClick($event, text: string = null) {
    const myText = (text) ? text : 'in:Cluster is:Node';
    const payload = {
      text: myText
    };
    this._controllerService.publish(Eventtypes.SEARCH_SET_SEARCHSTRING_REQUEST, payload);
    return false;
  }

  /**
   * This API serves html template. It invokes a SEARCH_OPEN_GUIDEDSERCH_REQUEST message.  Global search box will response to it.
   * @param $event
   */
  advanceSearchClick($event) {
    this._controllerService.publish(Eventtypes.SEARCH_OPEN_GUIDEDSERCH_REQUEST, {});
    return false;
  }

  /**
   * This function yields category title.
   * If there are more tenants in search-response, we display tenant information.
   * For exmaple
   * default-Cluster, audi-Cluster ( default and audi are tenants)
   */
  getCategoryTitle(catUIObj) {
    const tenants = this.searchSearchResponse['aggregated-entries'].tenants;
    const tenantKeys = Object.keys(tenants);

    return (tenantKeys.length > 1) ? catUIObj.tenant + '-' + catUIObj.name : catUIObj.name;
  }

  /**
   * This API serves html template.
   * @param category
   */
  getKinds(category): any {
    const kinds = [];
    const cat = category.value.kinds;
    const kindKeys = Object.keys(cat);
    for (let j = 0; j < kindKeys.length; j++) {
      const kind = cat[kindKeys[j]];
      const kindUIObj = {
        name: kindKeys[j],
        value: kind
      };
      kinds.push(kindUIObj);
    }
    if (!this.selectedKind) {
      this.selectedKind = kinds[0];
    }
    return kinds;
  }

  /**
   *
   * @param category
   */
  getCategoryCount(category) {
    const kinds = this.getKinds(category);
    let counts = 0;
    kinds.forEach(kind => {
      counts += kind.value.entries.length;
    });
    return counts;
  }

  /**
  * This API serves html.
  */
  getEntries(selectedKind): any {
    const entries = selectedKind.value.entries;
    return entries;
  }

  /**
   * This API serves html.
   * UI should fire an event and display object detail UI
   * @param $event
   * @param category
   * @param entry
   */
  onSearchResultEntryClick($event, category, entry) {
    const route = Utility.genSelfLinkRoute(entry.object.kind, entry.object.meta.name);
    this.router.navigateByUrl(route);
  }

  /**
   * This API serves html.
   * When user switches tab, we want to load category tab content. (load the right "kind")
   */
  onTabChange($event) {
    const selectedTabIndex = $event;
    this.selectedCategory = this.categories[selectedTabIndex];
    const kinds = this.getKinds(this.selectedCategory);
    if (kinds && kinds[0]) {
      this.selectedKind = kinds[0];
    }
  }


  /**
   * This API serves html template.
   * Display object.meta.label.
   * @param entry
   */
  displayLabels(entry): string {
    const list = [];
    if (entry.object.meta.labels != null) {
      return this.getObjectValues(entry.object.meta.labels);
    }
    return list.toString();
  }

  /**
   * This API serves html template.
   * Get displayable key for Event object
   * @param entry
   */
  getEventKeys(entry): string[] {
    const wantedKeys = [];
    Object.keys(entry.object).filter(key => {
      if (key !== 'meta' && key !== 'kind') {
        wantedKeys.push(key);
      }
    });
    return wantedKeys;
  }

  displayEventValue(event, eventValue): string {
    const value = Utility.getObjectValueByPropertyPath(new EventsEvent(event), eventValue);
    if (typeof value === 'string') {
      return value;
    }
    if (value instanceof Object) {
      return this.getObjectValues(new EventsEvent(event)[eventValue]);
    }
    return JSON.stringify(value);
  }

  private getObjectValues(eventValue: any): string {
    const list = [];
    const keys = Object.keys(eventValue);
    keys.forEach(key => {
      list.push(key + ' = ' + Utility.getObjectValueByPropertyPath(eventValue, [key]));
    });
    return list.join(', ');
  }

  getSearchInputErrors(): string[] {
    if (this._controllerService.LoginUserInfo) {
      const data: SearchResultPayload = this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA];
      if (data && data.searchstring) {
        const searchInput = data.searchstring;
        const compiled = SearchUtil.compileSearchInputString(searchInput);
        if (compiled.error) {
          return compiled.error.messages;
        }
      }
    }
    return [];
  }

  getOriginalSearchInput(): string {
    if (this._controllerService.LoginUserInfo) {
      const data = this._controllerService.LoginUserInfo[SearchUtil.LAST_SEARCH_DATA];
      if (data) {
        const searchInput = data['searched'];
        return searchInput;
      }
    }
    return null;
  }
}
