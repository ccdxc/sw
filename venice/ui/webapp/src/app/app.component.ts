import { Component, OnDestroy, OnInit, ViewEncapsulation, ViewChild, ElementRef, Directive, HostBinding } from '@angular/core';
import { OverlayContainer } from '@angular/cdk/overlay';
import { Store } from '@ngrx/store';
import { Subject } from 'rxjs/Subject';
import { takeUntil } from 'rxjs/operators/takeUntil';
import { map } from 'rxjs/operators/map';
import { filter } from 'rxjs/operators/filter';

import { login, logout } from '@app/core';
import { environment as env } from '@env/environment';

import { selectorSettings } from './components/settings';

import { ControllerService } from './services/controller.service';
import { DatafetchService } from './services/datafetch.service';
import { CommonComponent } from './common.component';

import { Eventtypes } from './enum/eventtypes.enum';
import { Logintypes } from './enum/logintypes.enum';
import { Utility } from './common/Utility';
import { MockDataUtil } from './common/MockDataUtil';
import { MatSidenav, MatSidenavContainer } from '@angular/material/sidenav';
import { LogService } from '@app/services/logging/log.service';


/**
 * This is the entry point component of Pensando-Venice Web-Application
 *
 */
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss', './app.component.sidenav.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AppComponent extends CommonComponent implements OnInit, OnDestroy {
  protected os = '';
  protected browsertype = '';
  protected browserversion = '';

  private subscriptions = {};
  private _currentComponent: any;

  // is Left-hand-side item click function registered?
  private _boolLSNItemCLKRegistered = false;

  protected isSideNavExpanded = true;

  // search
  searchVeniceApplication: any;
  searchVeniceApplicationsSuggestions: any = [];
  searchVeniceApplicationString: '';
  noSearchSuggestion: String = ' ';

  protected sidenavmenu: any = [
    {
      title: 'dashboard',
      children: []
    },
    {
      title: 'workload',
      children: []
    }

  ];

  @HostBinding('class') componentCssClass;
  private unsubscribeStore$: Subject<void> = new Subject<void>();
  @ViewChild('sidenav') _sidenav: MatSidenav;
  @ViewChild('rightSideNav') _rightSideNav: MatSidenav;
  @ViewChild('container') _container: MatSidenavContainer;

  protected _rightSivNavIndicator = 'notifications';

  constructor(
    protected _controllerService: ControllerService,
    protected _datafetchService: DatafetchService,
    protected _logService: LogService,
    public overlayContainer: OverlayContainer,
    private store: Store<any>,
  ) {
    super();

  }
  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to instantiate
  */
  ngOnInit() {
    Utility.getInstance().setControllerService(this._controllerService);
    Utility.getInstance().setLogService(this._logService);
    this.logger = Utility.getInstance().getLogService();

    this.os = Utility.getOperatingSystem();
    const browserObj = Utility.getBrowserInfomation();
    this.browsertype = browserObj['browserName'];
    this.browserversion = browserObj['browserName'] + browserObj['majorVersion'];

    this._subscribeToEvents();

    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    }

    this._bindtoStore();

  }



  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to exit
  */
  ngOnDestroy() {
    // this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { "component": "AppComponent", "state": Eventtypes.COMPONENT_DESTROY });
    this.unsubscribeStore$.next();
    this.unsubscribeStore$.complete();
    Object.keys(this.subscriptions).forEach((item) => {
      if (this.subscriptions[item]) {
        this.subscriptions[item].unsubscribe();
      }
    });
  }

  /**
   * Overide super's API
   * It will return AppComponent
   */
  getClassName(): string {
    return this.constructor.name;
  }

  private _bindtoStore() {
    this.store
      .select(selectorSettings)
      .pipe(
      takeUntil(this.unsubscribeStore$),
      map(({ theme }) => theme.toLowerCase())
      )
      .subscribe(theme => {
        this.componentCssClass = theme;
        this.overlayContainer.getContainerElement().classList.add(theme);
      });
  }

  onItemSelect(item) {
    this.log('AppComponent.onItemSelect() ' + item);
    this._controllerService.publish(Eventtypes.SIDENAV_INVOKATION_REQUEST, { 'id': item.title });
  }

  protected _registerSidenavMenuItemClick() {
    if (this._boolLSNItemCLKRegistered === true) {
      return;
    }
    const jQ = Utility.getJQuery();
    const self = this;
    jQ('.app-sidenav-item').on('click', function (event) {
      self._sideNavMenuItemClickHandler(event);
    });
    this._boolLSNItemCLKRegistered = true;
  }

  protected _sideNavMenuItemClickHandler(event) {
    this.log('APP _sideNavMenuItemClickHandler()', event.currentTarget.textContent.trim());
    let _invokeId = null;
    _invokeId = event.currentTarget.id;
    // (_invokeId=="_workload")
    if (_invokeId) {
      this._controllerService.publish(Eventtypes.SIDENAV_INVOKATION_REQUEST, { 'id': _invokeId });
    }
  }

  /**
   * Is the user logged in?
   */
  get isLoggedIn() {
    return this._controllerService.isUserLogin();
  }

  private _subscribeToEvents() {
    this.subscriptions[Eventtypes.LOGIN_FAILURE] = this._controllerService.subscribe(Eventtypes.LOGIN_FAILURE, (payload) => {

    });

    this.subscriptions[Eventtypes.COMPONENT_INIT] = this._controllerService.subscribe(Eventtypes.COMPONENT_INIT, (payload) => {
      this._handleComponentStateChangeInit(payload);
    });
    this.subscriptions[Eventtypes.COMPONENT_DESTROY] = this._controllerService.subscribe(Eventtypes.COMPONENT_DESTROY, (payload) => {
      this._handleComponentStateChangeDestroy(payload);
    });
  }

  _handleComponentStateChangeInit(payload: any) {
    this._currentComponent = payload;
    if (this._currentComponent['component'] !== 'LoginComponent') {
      this._registerSidenavMenuItemClick();
    }
  }

  _handleComponentStateChangeDestroy(payload: any) {

    this._currentComponent = null;
  }

  /**
   * This function serves html template.
   * It transforms the SideNav panel to other state
   * @param event
   */
  onSidebarCollapseClick(event) {
    this.isSideNavExpanded = !this.isSideNavExpanded;
    const jQuery = Utility.getJQuery();
    if (jQuery('#app-sidebar').hasClass('pindown') !== true) {
      jQuery('#app-sidebar').toggleClass('active');
      if (event && event.currentTarget) {
        jQuery(event.currentTarget).toggleClass('active');

        // very important, it trigger the autosize of sidenav-container
        setTimeout(() => {
          this._sidenav.open();
        }, 500);

      }
    }
  }

  /**
   * This function serves html template.
   * It pins down the SideNav panel to a fixed position
   * @param
   */
  onSidebarPindownClick($event) {
    const jQuery = Utility.getJQuery();

    jQuery('#app-sidebar').toggleClass('pindown');
    jQuery('#app-sidebarCollapse').toggleClass('pindown');
  }

  // search related functions START

  /**
   * This API serves HTML template. It will invoke getVeniceApplicationSearchSuggestions()
   * @param event
   */
  filterVeniceApplicationSearchSuggestions(event: any) {
    const value = event.query;
    this.searchVeniceApplicationsSuggestions = this.getVeniceApplicationSearchSuggestions(value);

  }

  /**
   * This API call server to fetch search suggestions
   * @param searched
   */
  protected getVeniceApplicationSearchSuggestions(searched: any) {
    this._datafetchService.globalSearch(searched).subscribe(
      data => {
        this._processGlobalSearchResult(searched, data);
      },
      err => {
        this.successMessage = '';
        this.errorMessage = 'Failed to get items! ' + err;
        this.error(err);
      }
    );
  }

  /**
   * This functions processes server provided search suggestions
   * @param searched
   * @param data
   *
   * TODO: using MockDataUtil to return s
   */
  protected _processGlobalSearchResult(searched, data) {
    // TODO: take out 'true' below
    if (true || !this.isRESTServerReady) {
      this.searchVeniceApplicationsSuggestions = MockDataUtil.getGlobalSearchResult(searched, data);
      if (this.searchVeniceApplicationsSuggestions && this.searchVeniceApplicationsSuggestions.length === 0) {
        this.searchVeniceApplicationString = searched;
        this.noSearchSuggestion = 'no search suggestion';
      }
    }
  }


  onAppHeaderSearchClick(event) {
    this.log('APP onAppHeaderSearchClick()', );
  }

  onSearchVeniceApplicationSelect(event) {
    this.log('APP onSearchVeniceApplicationSelect()', );
  }


  // search related functions END

  onLogoutClick() {
    this.store.dispatch(logout());
  }

  /**
   * This API serves html template
   * It tells the right-sideNav which ng-template to use
   */
  whichRightSideNav() {
    return this._rightSivNavIndicator;
  }

  /**
   * This function serves html template.
   * @param  $event
   * @param id
   */
  onToolbarIconClick($event, id) {

    if (this._rightSivNavIndicator === id) {
      this._rightSideNav.toggle();
    } else {
      this._rightSivNavIndicator = id;
      this._rightSideNav.open();
    }
    setTimeout(() => {
      // programmatically trigger window resize to tell sideNav container adjust widow size
        window.dispatchEvent(new Event('resize'));
    }, 500);

  }

}
