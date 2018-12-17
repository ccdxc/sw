/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component, DebugElement } from '@angular/core';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { MatIconRegistry } from '@angular/material';

/**-----
 Venice UI -  imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

import { SearchsuggestionsComponent } from './searchsuggestions.component';
import { MessageService } from 'primeng/primeng';

@Component({
  template: ''
})
class DummyComponent { }

describe('SearchsuggestionsComponent', () => {
  let component: SearchsuggestionsComponent;
  let fixture: ComponentFixture<SearchsuggestionsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [SearchsuggestionsComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule,
        MaterialdesignModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SearchsuggestionsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
