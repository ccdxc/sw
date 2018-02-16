/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed, inject } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { DebugElement } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { FormsModule } from '@angular/forms';
import { Observable } from 'rxjs/Observable';
import { combineAll } from 'rxjs/operator/combineAll';

/**-----
 VeniceUI Framework -  imports
 ------------------*/
import {WidgetsModule} from 'web-app-framework';

/**-----
 Venice UI -  imports
 ------------------*/
import { ControllerService } from '../../services/controller.service';
import { WorkloadService } from '../../services/workload.service';

import { WorkloadComponent } from './workload.component';

import { WorkloadwidgetComponent } from './workloadwidget/workloadwidget.component';
import { MockDataUtil } from '../../common/MockDataUtil';

/**-----
 Third-parties imports
 ------------------*/
import { MomentModule } from 'angular2-moment';

import {PrimengModule} from '@lib/primeng.module';
import { Component } from '@angular/core';
@Component( {
  template: ''
})
class DummyComponent { }

/**
 * workload.component.spec.ts
 * This is  UT test class of WorkloadComponent
 *
 * References:
 *     https://medium.com/google-developer-experts/angular-2-testing-guide-a485b6cb1ef0
 *     https://codecraft.tv/courses/angular/unit-testing/dependency-injection/ (http://plnkr.co/edit/PeWgLg6nbJ6MmdG8SzVW?p=preview)
 *     http://blog.danieleghidoli.it/2016/11/06/testing-angular-component-mock-services/
 *
 * How it works:
 *     Workload.component uses WorkloadService to call back-end (getItems(..) API) . In this Unit test, we use "MockWorkloadService"
 *     line-A: TestBed.overrideComponent(..) registers MockWorkloadService for WorkloadComponent
 *     line-B and Line-C retrieve read-service and mock-service
 *
 *     test-3 shows how to invoke the mock-service
 *        spy = spyOn(componentService, 'getItems').and.callThrough();  //Since we are using mock-service. using callThrough() is very important. See reference-3 above
 *        component.getItems();  //invoke the call component API.  Set debug breakpoint in WorkloadComponent.getItem(..).subscribe(..).data=> ..
 *
 *
 */
class MockWorkloadService extends WorkloadService {
  public getItems() {
    const items = MockDataUtil.getWorkloadItems();
    // make sure to return an Obervable as component is going to call subscribe(..) on observable
    return Observable.of(items);
  }
}

describe('WorkloadComponent', () => {
  let component: WorkloadComponent;
  let fixture: ComponentFixture<WorkloadComponent>;

  let testBedService: WorkloadService;
  let componentService: WorkloadService;
  let spy: any;


  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [WorkloadComponent,
        WorkloadwidgetComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        // VeniceUI Framework
        WidgetsModule,
        // Moment.js
        MomentModule,
        // primeNG.js
        PrimengModule
      ],
      providers: [
        ControllerService,
        WorkloadService
      ]
    });

  }));

  beforeEach(() => {
    // Configure the component with another set of Providers
    // line-A
    TestBed.overrideComponent(
      WorkloadComponent,
      { set: { providers: [{ provide: WorkloadService, useClass: MockWorkloadService }] } }
    );
    // fixture line must be after
    fixture = TestBed.createComponent(WorkloadComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();

    // line-B
    // WorkloadService provided to the TestBed.overrideComponent(..)
    testBedService = TestBed.get(WorkloadService);

    // line-C
    // WorkloadService provided by Component, (should return MockAuthService)
    componentService = fixture.debugElement.injector.get(WorkloadService);
  });


  afterEach(() => {
    testBedService = null;
    componentService = null;
    component = null;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  // test-1
  it('Service injected via inject(...) and TestBed.get(...) should be the same instance',
      inject([WorkloadService], (injectService: WorkloadService) => {
        expect(injectService).toBe(testBedService);
      })
  );

  // test-2
  it('Service injected via component should be and instance of MockAuthService', () => {
    expect(componentService instanceof MockWorkloadService).toBeTruthy();
  });

  // test-3
  it('should call getItems()', () => {
    spy = spyOn(componentService, 'getItems').and.callThrough();  // Since we are using mock-service. using callThrough() is very important. See reference-3 above
    component.getItems();  // invoke the call component API.
    expect(component.items).toBeDefined();
    expect(component.items.length).toBeGreaterThan(0);

    expect(componentService.getItems).toHaveBeenCalled();
  });

});
