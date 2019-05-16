import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { ToasterComponent, ToasterItemComponent } from './toaster.component';
import { MessageService } from '@app/services/message.service';

describe('ToasterComponent', () => {
  let component: ToasterComponent;
  let fixture: ComponentFixture<ToasterComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ToasterComponent, ToasterItemComponent],
      providers: [
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ToasterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
