import { MessageService as PMessageService, Message as PMessage } from 'primeng/primeng';
import { Injectable } from '@angular/core';
import { Subject } from 'rxjs';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';

export interface RemoveMessageReq {
  summary: string;
  // If detail is not given, it will only check
  // if the summary matches before removing
  detail?: string;
}

export interface Message extends PMessage {
  buttons?: ToolbarButton[];
}

@Injectable()
export class MessageService {
  private removeSource = new Subject<RemoveMessageReq>();
  private messageSource = new Subject<Message | Message[]>();
  private clearSource = new Subject<string>();

  messageObserver = this.messageSource.asObservable();
  clearObserver = this.clearSource.asObservable();
  removeObserver = this.removeSource.asObservable();

  add(message: Message) {
    if (message) {
      this.messageSource.next(message);
    }
  }

  addAll(messages: Message[]) {
    if (messages && messages.length) {
      this.messageSource.next(messages);
    }
  }

  clear(key?: string) {
    this.clearSource.next(key || null);
  }


  remove(req: RemoveMessageReq) {
    this.removeSource.next(req);
  }

}
