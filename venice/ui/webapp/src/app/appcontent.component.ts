import { OverlayContainer } from '@angular/cdk/overlay';
import { Component, HostBinding, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { MatDialog } from '@angular/material';
import { MatSidenav, MatSidenavContainer } from '@angular/material/sidenav';
import { logout } from '@app/core';
import { AlerttableService } from '@app/services/alerttable.service';
import { LogService } from '@app/services/logging/log.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { IdleWarningComponent } from '@app/widgets/idlewarning/idlewarning.component';
import { ToolbarComponent } from '@app/widgets/toolbar/toolbar.component';
import { DEFAULT_INTERRUPTSOURCES, Idle } from '@ng-idle/core';
import { Store } from '@ngrx/store';
import { map, takeUntil } from 'rxjs/operators';
import { Subject } from 'rxjs';
import { CommonComponent } from './common.component';
import { Utility } from './common/Utility';
import { selectorSettings } from './components/settings-group';
import { Eventtypes } from './enum/eventtypes.enum';
import { ControllerService } from './services/controller.service';


/**
 * This is the entry point component of Pensando-Venice Web-Application
 */
@Component({
  selector: 'app-content',
  templateUrl: './appcontent.component.html',
  styleUrls: ['./appcontent.component.scss', './appcontent.component.sidenav.scss', './appcontent.component.toast.scss'],
  encapsulation: ViewEncapsulation.None,
  providers: []
})
export class AppcontentComponent extends CommonComponent implements OnInit, OnDestroy {
  os = '';
  browsertype = '';
  browserversion = '';


  _currentComponent: any;

  // is Left-hand-side item click function registered?
  private _boolInitApp = false;

  protected isSideNavExpanded = true;


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
  @ViewChild('breadcrumbToolbar') _breadcrumbToolbar: ToolbarComponent;

  protected _rightSivNavIndicator = 'notifications';

  constructor(
    protected _controllerService: ControllerService,
    protected _logService: LogService,
    protected _alerttableSerivce: AlerttableService,
    public overlayContainer: OverlayContainer,
    private store: Store<any>,
    private idle: Idle,
    public dialog: MatDialog,
    protected uiconfigsService: UIConfigsService
  ) {
    super();
  }
  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to instantiate
  */
  ngOnInit() {
    this.logger = Utility.getInstance().getLogService();

    this.os = Utility.getOperatingSystem();
    const browserObj = Utility.getBrowserInfomation();
    this.browsertype = browserObj['browserName'];
    this.browserversion = browserObj['browserName'] + browserObj['majorVersion'];
    this._initAppData();

    this._subscribeToEvents();
    this._bindtoStore();
  }



  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to exit
  */
  ngOnDestroy() {
    this.unsubscribeStore$.next();
    this.unsubscribeStore$.complete();
    this.unsubscribeAll();
    this._boolInitApp = false;
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


  protected _registerSidenavMenuItemClick() {

    const jQ = Utility.getJQuery();
    const self = this;
    jQ('.app-sidenav-item').on('click', function(event) {
      self._sideNavMenuItemClickHandler(event);
    });

    jQ('a[aria-expanded]').on('click', function(event) {
      self._sideNavMenuGroupHeaderClickHandler(event);
    });

  }

  /**
   * Make the sideNav group function like an accordion. (only one group expand and all other collapse)
   * This is highly related to html make-up.
   */
  _sideNavMenuGroupHeaderClickHandler(event) {
    const $ = Utility.getJQuery();
    const groupheaders = $('a[aria-expanded]');
    for (let i = 0; i < groupheaders.length; i++) {
      if (groupheaders[i] !== event.currentTarget) {
        $(groupheaders[i]).attr('aria-expanded', false);
        if ($(groupheaders[i]).siblings('ul')) {
          $(groupheaders[i]).siblings('ul').removeClass('in');
        }
      }
    }
    this._resetHighlightedItems(event);
  }

  /**
   * Invoke Left-hand-side menu item selection
   * @param event
   */
  protected _sideNavMenuItemClickHandler(event) {
    this.log('APP _sideNavMenuItemClickHandler()', event.currentTarget.textContent.trim());
    let _invokeId = null;
    _invokeId = event.currentTarget.id;
    if (_invokeId) {
      this._controllerService.publish(Eventtypes.SIDENAV_INVOKATION_REQUEST, { 'id': _invokeId });
    }
    this._resetHighlightedItems(event);
  }

  /**
   * Removes previously highlighted item -- say 'workload' was highlighted, now user clicks 'security group'. 'workload' should be reset.
   */
  private _resetHighlightedItems(event: any) {
    if (!event) {
      return;
    }
    const $ = Utility.getJQuery();
    const list = $('.app-sidenav-selected');
    for (let i = 0; i < list.length; i++) {
      if (list[i] !== event.currentTarget) {
        $(list[i]).removeClass('app-sidenav-selected');
      }
    }
    $(event.currentTarget).addClass('app-sidenav-selected');
  }

  /***** Routing *****/

  public navigate(paths: string[]) {
    this._controllerService.navigate(paths);
  }

  /**
   * handles case when user logout event occurs
   */
  private onLogout(payload: any) {
    this._controllerService.LoginUserInfo = null;
    this._boolInitApp = false;
    this.navigate(['/login']);
  }

  /**
   * Routing the sidebar navigation
   */
  private onSidenavInvokation(payload) {
    if (payload['id'] === 'workload') {
      this.navigate(['/workload']);
    } else if (payload['id'] === 'sgpolicies') {
      this.navigate(['/security', 'sgpolicies']);
    } else if (payload['id'] === 'network') {
      this.navigate(['/network', 'network']);
    } else if (payload['id'] === 'troubleshooting') {
      this.navigate(['/monitoring', 'troubleshooting']);
    } else if (payload['id'] === 'cluster') {
      this.navigate(['/cluster', 'cluster']);
    } else if (payload['id'] === 'naples') {
      this.navigate(['/cluster', 'naples']);
    } else if (payload['id'] === 'alertsevents') {
      this.navigate(['/monitoring', 'alertsevents']);
    } else {
      this.uiconfigsService.navigateToHomepage();
    }
  }

  private onSearchResultReady(payload) {
    if (payload['id'] === 'searchresult') {
      this.navigate(['/searchresult']);
    } else {
      console.error(this.getClassName() + '.onSearchResultReady() payload.id not supported \n' + payload);
    }
  }


  private _subscribeToEvents() {


    this.subscriptions[Eventtypes.IDLE_CHANGE] = this._controllerService.subscribe(Eventtypes.IDLE_CHANGE, (payload) => {
      this.handleIdleChange(payload);
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
    this.subscriptions[Eventtypes.SEARCH_RESULT_LOAD_REQUEST] = this._controllerService.subscribe(Eventtypes.SEARCH_RESULT_LOAD_REQUEST, (payload) => {
      this.onSearchResultReady(payload);
    });
  }

  _handleComponentStateChangeInit(payload: any) {
    this._currentComponent = payload;
  }

  _initAppData() {
    if (this._boolInitApp === true) {
      return;
    } else {
      this._registerSidenavMenuItemClick();
      this.getAlerts();
    }
    this._boolInitApp = true;
    this._setupIdle();
  }

  _setupIdle() {
    this.idle.setIdle(5);
    this.idle.setTimeout(10);
    this.idle.setInterrupts(DEFAULT_INTERRUPTSOURCES);
    this.idle.onIdleEnd.subscribe(() => { this.showIdleWarning = false; });
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
          data: { countdown: countdown }
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
    if (this._breadcrumbToolbar) {
      this._breadcrumbToolbar.clear();
    }
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
    this.navigate(['/monitoring', 'alertsevents']);
    this._rightSideNav.close();

  }

  settingsNavigate(route) {
    this._rightSideNav.close();
    this.navigate(route);
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
