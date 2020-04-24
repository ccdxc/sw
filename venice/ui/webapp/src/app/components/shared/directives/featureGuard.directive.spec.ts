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
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { AuthService } from '@app/services/auth.service';
import { FeatureGuardDirective } from './featureGuard.directive';


@Component({
   template: `<div id="testcontainer">
                <!-- Full syntax required -->
                <div *featureGuard="{req: ['feature1', 'feature2']}" id='div1'></div>

                <!-- Full syntax optional -->
                <div *featureGuard="{opt: ['feature3', 'feature4']}" id='div2'></div>

                <!-- shorthand syntax 1 -->
                <div *featureGuard="{req: 'feature3' }" id='div3'></div>

                <!-- shorthand syntax 2 -->
                <div *featureGuard="'feature3'" id='div4'></div>

                <!-- inverse and else template -->
                <div *featureGuard="{req: 'feature3', checkUnauthorized: true }; else elseTemplate" id='div5'></div>


                <ng-template #elseTemplate>
                  <div id='div5Else'></div>
                </ng-template>

                <!-- Empty guard should always show content-->
                <div *featureGuard='' id='div6'></div>
                `,
})
class DummyComponent { }

interface TestCase {
  description: string;
  features: string[];
  expectedDivs: string[];
}

describe('featureGuard directive', () => {
  let component: DummyComponent;
  let fixture: ComponentFixture<DummyComponent>;
  let uiConfigs: UIConfigsService;
  let controllerService: ControllerService;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        DummyComponent,
        FeatureGuardDirective
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
        AuthService,
        LicenseService,
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
        features: [ 'feature1', 'feature2', 'feature3', 'feature4'],
        expectedDivs: [ 'div1', 'div2', 'div3', 'div4', 'div5Else', 'div6']
      },
      {
        description: 'Missing one required property, shouldn\'t show',
        features: [ 'feature1', 'feature3', 'feature4'],
        expectedDivs: [ 'div2', 'div3', 'div4', 'div5Else', 'div6']
      },
      {
        description: 'Optional should show even if its missing one, Else template shouldn\'t show',
        features: [ 'feature4'],
        expectedDivs: [ 'div2',  'div5', 'div6']
      },
    ];

    testCases.forEach( (tc, index) => {
      // Casting to any so we can access private variable during testing
      const anyService = uiConfigs as any;
      anyService.configFile = {
        'enabled-features': tc.features
      };

      if (index !== 0) {
        // If not the first time, we need to emit event for the guard to recompute
        controllerService.publish(Eventtypes.NEW_FEATURE_PERMISSIONS, null);
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

