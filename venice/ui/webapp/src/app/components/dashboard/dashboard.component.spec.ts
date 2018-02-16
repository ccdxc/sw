import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { DebugElement } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { Component } from '@angular/core';

import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule } from '@angular/common/http/testing';

import { GridsterModule } from 'angular2gridster';
import {WidgetsModule} from 'web-app-framework';
import {PrimengModule} from '@lib/primeng.module';

import { FormsModule } from '@angular/forms';
import { ControllerService } from '../../services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { DashboardComponent } from './dashboard.component';

@Component( {
  template: ''
})
class DummyComponent { }

describe('DashboardComponent', () => {
  let component: DashboardComponent;
  let fixture: ComponentFixture<DashboardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DashboardComponent, DummyComponent ],
      imports: [

          RouterTestingModule.withRoutes([
            { path: 'login', component: DummyComponent }
          ]),
          FormsModule,
          HttpClientTestingModule,
          GridsterModule,
          WidgetsModule,
          PrimengModule
        ],
      providers: [
          ControllerService,
          LogService,
          LogPublishersService
        ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
