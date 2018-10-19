import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ToasterComponent, ToasterItemComponent } from './toaster.component';
import { MessageService } from 'primeng/primeng';

describe('ToasterComponent', () => {
  let component: ToasterComponent;
  let fixture: ComponentFixture<ToasterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ToasterComponent, ToasterItemComponent],
      providers: [
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ToasterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
