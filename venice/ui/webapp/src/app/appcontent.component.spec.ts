/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { FormsModule } from '@angular/forms';
import { OverlayContainer } from '@angular/cdk/overlay';


import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';

import { Store } from '@ngrx/store';
/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { ControllerService } from './services/controller.service';

import { LogService } from './services/logging/log.service';
import { LogPublishersService } from './services/logging/log-publishers.service';
import { AuthService } from './services/auth.service';
import { AuthService as AuthServiceGen } from './services/generated/auth.service';
import { CoreModule } from '@app/core';
import { AlertlistModule } from '@app/components/alertlist';
import { LoginModule } from './components/login/login.module';
import { ToolbarComponent } from './widgets/toolbar/toolbar.component';
import { TestingUtility } from '@app/common/TestingUtility';
import { MonitoringService } from '@app/services/generated/monitoring.service';


/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from './lib/primeng.module';
import { MaterialdesignModule } from './lib/materialdesign.module';
import { NgIdleKeepaliveModule } from '@ng-idle/keepalive';
import { AlerttableService } from '@app/services/alerttable.service';


import { SearchComponent } from '@app/components/search/search/search.component';
import { SearchboxComponent } from '@app/components/search/searchbox/searchbox.component';
import { SearchsuggestionsComponent } from '@app/components/search/searchsuggestions/searchsuggestions.component';
import { GuidesearchComponent } from '@app/components/search/guidedsearch/guidedsearch.component';

import { AppcontentComponent } from '@app/appcontent.component';
import { SearchService } from '@app/services/generated/search.service';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SharedModule } from '@app/components/shared/shared.module';
import { ToasterComponent, ToasterItemComponent } from '@app/widgets/toaster/toaster.component';
import { ConfirmationService } from 'primeng/primeng';
import { ClusterService } from './services/generated/cluster.service';
import { MessageService } from './services/message.service';
import { HelpoverlayComponent } from './widgets/helpcontent/helpoverlay.component';
import { PortalModule } from '@angular/cdk/portal';
import { BgfileuploadComponent } from '@app/widgets/bgfileupload/bgfileupload.component';
import { MonitoringAlert } from '@sdk/v1/models/generated/monitoring';
import { RolloutService } from '@app/services/generated/rollout.service';
import { RolloutRolloutStatus_state } from '@sdk/v1/models/generated/rollout';
import { DebugElement } from '@angular/core';
import { By } from '@angular/platform-browser';
import { BehaviorSubject, Observable } from 'rxjs';
import { Router } from '@angular/router';
import { Utility } from './common/Utility';
import { ISearchSearchResponse } from '@sdk/v1/models/generated/search';
import { WorkloadService } from './services/generated/workload.service';

describe('AppcontentComponent', () => {
  let component: AppcontentComponent;
  let fixture: ComponentFixture<AppcontentComponent>;

    configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppcontentComponent,
        ToolbarComponent,
        SearchComponent,
        SearchboxComponent,
        SearchsuggestionsComponent,
        GuidesearchComponent,
        ToasterComponent,
        ToasterItemComponent,
        HelpoverlayComponent,
        BgfileuploadComponent
      ],
      imports: [
        // Other modules...
        HttpClientTestingModule,
        RouterTestingModule,
        FormsModule,
        PrimengModule,
        MaterialdesignModule,
        WidgetsModule,
        CoreModule,
        AlertlistModule,
        LoginModule,
        NgIdleKeepaliveModule.forRoot(),
        NoopAnimationsModule,
        SharedModule,
        PortalModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        AlerttableService,
        AuthService,
        MonitoringService,
        LogService,
        LogPublishersService,
        Store,
        SearchService,
        OverlayContainer,
        UIConfigsService,
        AuthServiceGen,
        MessageService,
        ClusterService,
        RolloutService,
        WorkloadService
      ],
    });

    });

  beforeEach(() => {
    fixture = TestBed.createComponent(AppcontentComponent);
    component = fixture.componentInstance;
  });

  it('should create the app', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  it('should show new alerts that come in', () => {
    const alerts = [];
    for (let index = 0; index < 20; index++) {
      const alert = new MonitoringAlert();
      alert.meta.name = 'alert' + index;
      alerts.push(alert);
    }
    // Get will return all 20
    // Watch will return in three batches, 10, 5, 5
    // We then send a new alert to watch, which should trigger a notification
    const controllerService = TestBed.get(ControllerService);
    const toasterSpy = spyOn(controllerService, 'invokeInfoToaster');

    const monitoringService = TestBed.get(MonitoringService);
    const searchService = TestBed.get(SearchService);
    const searchResp: ISearchSearchResponse = {
      'total-hits': alerts.length.toString(),
    };
    spyOn(searchService, 'PostQuery').and.returnValue(
      new BehaviorSubject({
        body: searchResp
      })
    );
    const watchSubject = TestingUtility.createWatchEventsSubject(alerts.slice(0, 10));
    spyOn(monitoringService, 'WatchAlert').and.returnValue(
      watchSubject
    );

    fixture.detectChanges();
    expect(toasterSpy).toHaveBeenCalledTimes(0);

    watchSubject.next(TestingUtility.createWatchEvents(alerts.slice(10, 15)));
    fixture.detectChanges();
    expect(toasterSpy).toHaveBeenCalledTimes(0);

    watchSubject.next(TestingUtility.createWatchEvents(alerts.slice(15)));
    fixture.detectChanges();
    expect(toasterSpy).toHaveBeenCalledTimes(0);

    const newAlerts = [];
    for (let index = 0; index < 5; index++) {
      const alert = new MonitoringAlert();
      alert.meta.name = 'newalert' + index;
      newAlerts.push(alert);
    }

    // New alerts
    watchSubject.next(TestingUtility.createWatchEvents(newAlerts));
    fixture.detectChanges();
    expect(toasterSpy).toHaveBeenCalledTimes(1);

  });

});

describe('UIBlock during rollout progress', () => {
  let fixture: ComponentFixture<AppcontentComponent>;
  let debugElement: DebugElement;
  let clusterService: ClusterService;
  let rolloutService: RolloutService;
  const mockRouter = {
    events: new Observable(observer => {
      observer.next();
      observer.complete();
    }),
    navigate: jasmine.createSpy('navigate')
  };
  const versionObj = {events: [
    {
      object: {
        kind: 'Version',
        'api-version': 'v1',
        meta: {
          name: 'clusterVersion'
        },
        spec: {},
        status: {
          'build-date': '2019-06-19T16:34:11-0700',
          'build-version': '0.11.0-55',
          'vcs-commit': '3de49898626af6646b03800ebbe048e81f67bbe6',
          'rollout-build-version': 'a'
        },
        type: 'Created'
      },
      type: 'Created'
    }
  ]};
  const rolloutObj = {'events': [
    { 'type': 'Created',
      'object': {
        'kind'        : 'Rollout',
        'api-version' : 'v1',
        'meta'        : {
          'name'      : 'a1'},
        'spec'        : {},
        'status'      : {
          'state'     : RolloutRolloutStatus_state.progressing }
        }
      }
    ]
  };


  configureTestSuite(() => {
  TestBed.configureTestingModule({
    declarations: [
      AppcontentComponent,
      ToolbarComponent,
      SearchComponent,
      SearchboxComponent,
      SearchsuggestionsComponent,
      GuidesearchComponent,
      ToasterComponent,
      ToasterItemComponent,
      HelpoverlayComponent,
      BgfileuploadComponent
    ],
    imports: [
      // Other modules...
      HttpClientTestingModule,
      RouterTestingModule,
      FormsModule,
      PrimengModule,
      MaterialdesignModule,
      WidgetsModule,
      CoreModule,
      AlertlistModule,
      LoginModule,
      NgIdleKeepaliveModule.forRoot(),
      NoopAnimationsModule,
      SharedModule,
      PortalModule
    ],
    providers: [
      ControllerService,
      ConfirmationService,
      AlerttableService,
      AuthService,
      MonitoringService,
      LogService,
      LogPublishersService,
      Store,
      SearchService,
      OverlayContainer,
      UIConfigsService,
      AuthServiceGen,
      MessageService,
      ClusterService,
      RolloutService,
      WorkloadService,
      {provide: Router, useValue: mockRouter}
    ],
  });

  });

  beforeEach(() => {
    fixture = TestBed.createComponent(AppcontentComponent);
    debugElement = fixture.debugElement;
    clusterService = TestBed.get(ClusterService);
    rolloutService = TestBed.get(RolloutService);
  });

  afterEach(() => {
    Utility.getInstance().setMaintenanceMode(false);
  });

  it('Rollout Progressing: Admin behaviour', () => {
    TestingUtility.setAllPermissions();
    spyOn(clusterService, 'WatchVersion').and.returnValue(
      new BehaviorSubject(versionObj)
    );
    spyOn(rolloutService, 'WatchRollout').and.returnValue(
      new BehaviorSubject(rolloutObj)
    );
    fixture.detectChanges();
    // check if sidenav menu is served
    expect(debugElement.query(By.css('mat-sidenav.app-shell-left-sidenav')).children[0].children[0].children.length).toBe(1);
    // check if redirected to progressing rollout page
    expect (mockRouter.navigate).toHaveBeenCalledWith (['/maintenance']);
  });

  it('Rollout Progressing: NonAdmin behaviour', () => {
    TestingUtility.removeAllPermissions();
    spyOn(clusterService, 'WatchVersion').and.returnValue(
      new BehaviorSubject(versionObj)
    );
    fixture.detectChanges();
    // check if sidenav menu is served
    expect(debugElement.query(By.css('mat-sidenav.app-shell-left-sidenav')).children[0].children[0].children.length).toBe(1);
    // check if uiblock is up
    expect(debugElement.query(By.css('div.ui-blockui'))).toBeTruthy();
  });
});
