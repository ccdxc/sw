import { OverlayContainer } from '@angular/cdk/overlay';
import { Component, HostBinding, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { MatDialog } from '@angular/material';
import { MatSidenav, MatSidenavContainer } from '@angular/material/sidenav';
import { logout } from '@app/core';
import { LogService } from '@app/services/logging/log.service';
import { IdleWarningComponent } from '@app/widgets/idlewarning/idlewarning.component';
import { DEFAULT_INTERRUPTSOURCES, Idle } from '@ng-idle/core';
import { Store } from '@ngrx/store';
import { map } from 'rxjs/operators/map';
import { takeUntil } from 'rxjs/operators/takeUntil';
import { Subject } from 'rxjs/Subject';

import { CommonComponent } from './common.component';
import { MockDataUtil } from './common/MockDataUtil';
import { Utility } from './common/Utility';
import { selectorSettings } from './components/settings';
import { Eventtypes } from './enum/eventtypes.enum';
import { ControllerService } from './services/controller.service';
import { DatafetchService } from './services/datafetch.service';
import { AlerttableService } from '@app/services/alerttable.service';


/**
 * This is the entry point component of Pensando-Venice Web-Application
 */
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss', './app.component.sidenav.scss'],
  encapsulation: ViewEncapsulation.None,
  providers: []
})
export class AppComponent extends CommonComponent implements OnInit, OnDestroy {
  protected os = '';
  protected browsertype = '';
  protected browserversion = '';

  private subscriptions = {};
  private _currentComponent: any;

  // is Left-hand-side item click function registered?
  private _boolIniApp = false;

  protected isSideNavExpanded = true;

  // search
  searchVeniceApplication: any;
  searchVeniceApplicationsSuggestions: any = [];
  searchVeniceApplicationString: '';
  noSearchSuggestion: String = ' ';

  // alerts
  alerts = [];
  alertNumbers = 0;

  // idling
  showIdleWarning = false;
  idleDialogRef: any;

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
    protected _alerttableSerivce: AlerttableService,
    public overlayContainer: OverlayContainer,
    private store: Store<any>,
    private idle: Idle,
    public dialog: MatDialog,
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

    const jQ = Utility.getJQuery();
    const self = this;
    jQ('.app-sidenav-item').on('click', function (event) {
      self._sideNavMenuItemClickHandler(event);
    });

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

  /***** Routing *****/

  public navigate(path: string) {
    this._controllerService.navigate(path);
  }

  /**
   * handles case when user login is successful.
   */
  private onLogin(payload: any) {
    this._controllerService.LoginUserInfo = payload['data'];
    this._controllerService.directPageAsUserAlreadyLogin();
  }

  /**
  * handles case when we find user not yet login.
  * For example, before login, user can change browser URL to access page. But we want to block it.
  */
  private onNotYetLogin(payload: any) {
    this.navigate('/login');
  }

  /**
   * handles case when user logout event occurs
   */
  private onLogout(payload: any) {
    this._controllerService.LoginUserInfo = null;
    this.navigate('/login');
  }


  /**
   * Routing the sidebar navigation
   */
  private onSidenavInvokation(payload) {
    if (payload['id'] === 'workload') {
      this.navigate('/workload');
    } else if (payload['id'] === 'alerttable') {
      this.navigate('/alerttable');
    } else {
      this.navigate('/dashboard');
    }
  }

  private _subscribeToEvents() {
    // setting up route watching
    this.subscriptions[Eventtypes.LOGIN_FAILURE] = this._controllerService.subscribe(Eventtypes.LOGIN_FAILURE, (payload) => {

    });

    this.subscriptions[Eventtypes.IDLE_CHANGE] = this._controllerService.subscribe(Eventtypes.IDLE_CHANGE, (payload) => {
      this.handleIdleChange(payload);
    });

    this.subscriptions[Eventtypes.LOGIN_SUCCESS] = this._controllerService.subscribe(Eventtypes.LOGIN_SUCCESS, (payload) => {
      this.onLogin(payload);
    });

    this.subscriptions[Eventtypes.NOT_YET_LOGIN] = this._controllerService.subscribe(Eventtypes.NOT_YET_LOGIN, (payload) => {
      this.onNotYetLogin(payload);
    });

    this.subscriptions[Eventtypes.LOGOUT] = this._controllerService.subscribe(Eventtypes.LOGOUT, (payload) => {
      this.onLogout(payload);
    });

    this.subscriptions[Eventtypes.SIDENAV_INVOKATION_REQUEST] = this._controllerService.subscribe(Eventtypes.SIDENAV_INVOKATION_REQUEST, (payload) => {
      this.onSidenavInvokation(payload);
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
      this._initAppData();
    }
  }

  _initAppData() {
    if (this._boolIniApp === true) {
      return;
    } else {
      this._registerSidenavMenuItemClick();
      this.getAlerts();
    }
    this._boolIniApp = true;
    this._setupIdle();
  }

  _setupIdle() {
    this.idle.setIdle(5);
    this.idle.setTimeout(10);
    this.idle.setInterrupts(DEFAULT_INTERRUPTSOURCES);
    this.idle.onIdleEnd.subscribe(() => {this.showIdleWarning = false; });
    this.idle.onTimeout.subscribe(() => {
      this.idleDialogRef.close();
      this.onLogoutClick();
    });
    this.idle.onTimeoutWarning.subscribe((countdown) => {
      if (!this.showIdleWarning) {
        this.showIdleWarning = true;
        this.idleDialogRef = this.dialog.open(IdleWarningComponent, {
          width: '400px',
          hasBackdrop: true,
          data: { countdown: countdown}
        });
      } else {
        this.idleDialogRef.componentInstance.updateCountdown(countdown);
      }
    });
  }

  handleIdleChange(payload: any) {
    if (payload.active) {
      this.idle.watch();
    } else {
      this.idle.stop();
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

  /**
   * Call server to fetch all alerts to populate RHS alert-list
   */
  getAlerts() {
    const payload = '';
    this._alerttableSerivce.getAlertList(payload).subscribe(
      data => {
        this.alerts = data;
        this.alertNumbers = this.alerts.length;
      },
      err => {
        this.successMessage = '';
        this.errorMessage = 'Failed to get items! ' + err;
        this.error(err);
      }
    );
  }

  /**
   * This API serves html template
   * It response to user request of expanding all alerts (in RHS alert-list panel)
   * @param  alertlist
   */
  onExpandAllAlertsClick(alertlist) {
    console.log('AppComponet.onExpandAllAlertsClick()');
    // this._controllerService.publish(Eventtypes.SIDENAV_INVOKATION_REQUEST, { 'id': 'alerttable' });
    this.navigate('alerttable');
    this._rightSideNav.close();

  }

  pocBuildChart($event) {
    const baseComponent = this._controllerService.instantiateComponent('BaseComponent');
    this._controllerService.appendComponentToDOMElement(baseComponent, document.body);

    this._controllerService.buildComponentFromModule('@components/workload/workload.module#WorkloadModule', 'WorkloadwidgetComponent')
      .then((component) => {
        // change component properties and append component to UI view
        this._componentSetup(component);
        this._controllerService.appendComponentToDOMElement(component, document.body);
      });
  }

  pocGetAlerts($event) {
    this.getAlerts();
    this._rightSivNavIndicator = 'notifications';
  }

  _componentSetup(component) {
    component.instance.style_class = 'wlWidget-positive';
    component.instance.color = '#7db1ea';
    component.instance.id = 'test_id';
    component.instance.title = 'Test';
    component.instance.label = '10';
    component.instance.styleType = 'positive';
    component.icon = {
      margin: {
        top: '5px',
        right: '5px',
        url: 'test.com'
      }
    };
    const dataset = {
      x: [1, 2, 3, 4, 5],
      y: [1, 3, 2, 3, 8]
    };

    component.instance.data = dataset;
    component.instance.updateData();

  }

}
