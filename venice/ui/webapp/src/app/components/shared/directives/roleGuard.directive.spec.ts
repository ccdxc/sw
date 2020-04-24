/**-----
 Angular imports
 ------------------*/
 import { HttpClientTestingModule } from '@angular/common/http/testing';
 import { Component, ViewContainerRef } from '@angular/core';
 import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
 import { RouterTestingModule } from '@angular/router/testing';
 /**-----
  Venice web-app imports
  ------------------*/
 import { ControllerService } from '@app/services/controller.service';
 import { ConfirmationService } from 'primeng/primeng';
 import { LogPublishersService } from '@app/services/logging/log-publishers.service';
 import { LogService } from '@app/services/logging/log.service';
 import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
 import { By } from '@angular/platform-browser';
 import { MessageService } from '@app/services/message.service';
import { RoleGuardDirective } from './roleGuard.directive';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { AuthService } from '@app/services/auth.service';


@Component({
   template: `<div id="testcontainer">
                <!-- Full syntax required -->
                <div *roleGuard="{req: ['workload_read', 'workload_create']}" id='div1'></div>

                <!-- Full syntax optional -->
                <div *roleGuard="{opt: ['alert_read', 'event_read']}" id='div2'></div>

                <!-- shorthand syntax 1 -->
                <div *roleGuard="{req: 'alert_read' }" id='div3'></div>

                <!-- shorthand syntax 2 -->
                <div *roleGuard="'alert_read'" id='div4'></div>

                <!-- inverse and else template -->
                <div *roleGuard="{req: 'alert_read', checkUnauthorized: true }; else elseTemplate" id='div5'></div>


                <ng-template #elseTemplate>
                  <div id='div5Else'></div>
                </ng-template>

                <!-- Empty guard should always show content-->
                <div *roleGuard='' id='div6'></div>
                `,
})
class DummyComponent { }

interface TestCase {
  description: string;
  uiPermissions: string[];
  expectedDivs: string[];
}

describe('roleGuard directive', () => {
  let component: DummyComponent;
  let fixture: ComponentFixture<DummyComponent>;
  let uiConfigs: UIConfigsService;
  let controllerService: ControllerService;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        DummyComponent,
        RoleGuardDirective
      ],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        UIConfigsService,
        LicenseService,
        MessageService,
        ViewContainerRef,
        AuthService
      ]
    });
      });

  beforeEach(() => {
    uiConfigs = TestBed.get(UIConfigsService);
    controllerService = TestBed.get(ControllerService);
    fixture = TestBed.createComponent(DummyComponent);
    component = fixture.componentInstance;
  });

  it('Should hide html elements based on permissions', () => {
    const testCases: TestCase[] = [
      {
        description: 'All should be visible, with else template',
        uiPermissions: [ 'workload_read', 'workload_create', 'alert_read', 'event_read'],
        expectedDivs: [ 'div1', 'div2', 'div3', 'div4', 'div5Else', 'div6']
      },
      {
        description: 'Missing one required property, shouldn\'t show',
        uiPermissions: [ 'workload_read', 'alert_read', 'event_read'],
        expectedDivs: [ 'div2', 'div3', 'div4', 'div5Else', 'div6']
      },
      {
        description: 'Optional should show even if its missing one, Else template shouldn\'t show',
        uiPermissions: [ 'event_read'],
        expectedDivs: [ 'div2',  'div5', 'div6']
      },
    ];

    testCases.forEach( (tc, index) => {
      const permissionMap = {};
      tc.uiPermissions.forEach( key => {
        permissionMap[key] = true;
      });
      // Casting to any so we can access private variable during testing
      const anyService = uiConfigs as any;
      anyService.uiPermissions = permissionMap;

      if (index !== 0) {
        // If not the first time, we need to emit event for the guard to recompute
        controllerService.publish(Eventtypes.NEW_USER_PERMISSIONS, null);
      }
      fixture.detectChanges();
      const container = fixture.debugElement.query(By.css('#testcontainer'));
      expect(container).toBeTruthy(tc.description + ' Top level container div is missing');
      expect(container.queryAll(By.css('div')).length).toBe(tc.expectedDivs.length, tc.description);
      tc.expectedDivs.forEach( id => {
        expect(By.css('#' + id)).toBeTruthy(tc.description);
      });
    });
  });
});

